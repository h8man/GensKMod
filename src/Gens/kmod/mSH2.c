#include <windows.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../Mem_SH2.h"
#include "../SH2.h"
#include "../SH2D.h"
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "mSH2.h"
#include "hexbox.h"
#include "dasmbox.h"

static HWND hMSH2;
static unsigned char MSH2_ViewMode;
static unsigned int  MSH2_StartLineROMDisasm, MSH2_StartLineRAMDisasm, MSH2_StartLineRAM, MSH2_StartLineROM, MSH2_StartLineCache;
static CHAR debug_string[1024];

static BOOL MSH2_IsDisasmView(void)
{
	return ((MSH2_ViewMode & 2) && (MSH2_ViewMode & 1)) ||
		((MSH2_ViewMode & 8) && (MSH2_ViewMode & 4));
}

static BOOL CALLBACK GetMSH2DasmInstruction(void* context, DWORD address, DWORD* nextAddress, LPSTR text, UINT textCapacity)
{
	char instruction[DASMBOX_TEXT_CAPACITY];
	unsigned int startAddress, endAddress;
	(void)context;

	if (MSH2_ViewMode & 2)
	{
		startAddress = 0x02000000;
		endAddress = 0x02400000;
	}
	else
	{
		startAddress = 0x06000000;
		endAddress = 0x06040000;
	}
	if (address < startAddress || address > endAddress - 2)
		return FALSE;

	SH2Disasm(instruction, address, SH2_Read_Word(&M_SH2, address), 0);
	instruction[DASMBOX_TEXT_CAPACITY - 1] = 0;
	*nextAddress = address + 2;
	lstrcpynA(text, instruction + 8, textCapacity);
	return TRUE;
}

static void JumpMSH2To(DWORD address)
{
	if (MSH2_IsDisasmView())
		SendDlgItemMessage(hMSH2, IDC_MSH2_DASMBOX, DASMBOX_GOTO_ADDRESS, (WPARAM)address, 0);
	else
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_GOTO_ADDRESS, (WPARAM)address, 0);
}

static void RestoreMSH2Position(void)
{
	if ((MSH2_ViewMode & 2) && (MSH2_ViewMode & 1))
		JumpMSH2To(MSH2_StartLineROMDisasm);
	else if ((MSH2_ViewMode & 8) && (MSH2_ViewMode & 4))
		JumpMSH2To(MSH2_StartLineRAMDisasm);
	else if (MSH2_ViewMode & 2)
		JumpMSH2To(0x02000000 + MSH2_StartLineROM * 8);
	else if (MSH2_ViewMode & 8)
		JumpMSH2To(0x06000000 + MSH2_StartLineRAM * 8);
	else if (MSH2_ViewMode & 0x20)
		JumpMSH2To(0xC0000000 + MSH2_StartLineCache * 8);
}

void UpdateMSH2_KMod()
{
	DASMBOX_SOURCE dasmSource;

	if ((MSH2_ViewMode & 2) && (MSH2_ViewMode & 1))
	{
		dasmSource.lineCount = 0x02400000;
		dasmSource.addressLength = 8;
		dasmSource.startAddress = 0x02000000;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetMSH2DasmInstruction;
		SendDlgItemMessage(hMSH2, IDC_MSH2_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if ((MSH2_ViewMode & 8) && (MSH2_ViewMode & 4))
	{
		dasmSource.lineCount = 0x06040000;
		dasmSource.addressLength = 8;
		dasmSource.startAddress = 0x06000000;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetMSH2DasmInstruction;
		SendDlgItemMessage(hMSH2, IDC_MSH2_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if (MSH2_ViewMode & 2)
	{
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(_32X_Rom), (LPARAM)_32X_Rom);
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 8, HEXBOX_MODE_BYTE), (LPARAM)0x02000000);
	}
	else if (MSH2_ViewMode & 8)
	{
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(_32X_Ram), (LPARAM)_32X_Ram);
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 8, HEXBOX_MODE_BYTE), (LPARAM)0x06000000);
	}
	else if (MSH2_ViewMode & 0x20)
	{
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(M_SH2.Cache), (LPARAM)M_SH2.Cache);
		SendDlgItemMessage(hMSH2, IDC_MSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 8, HEXBOX_MODE_BYTE), (LPARAM)0xC0000000);
	}
	wsprintf(debug_string, "T=%d S=%d Q=%d M=%d I=%.1X SR=%.4X Status=%.4X", SH2_Get_SR(&M_SH2) & 1, (SH2_Get_SR(&M_SH2) >> 1) & 1, (SH2_Get_SR(&M_SH2) >> 8) & 1, (SH2_Get_SR(&M_SH2) >> 9) & 1, (SH2_Get_SR(&M_SH2) >> 4) & 0xF, SH2_Get_SR(&M_SH2), M_SH2.Status & 0xFFFF);
	SendDlgItemMessage(hMSH2, IDC_MSH2_STATUS_SR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "R0=%.8X R1=%.8X R2=%.8X R3=%.8X\nR4=%.8X R5=%.8X R6=%.8X R7=%.8X\nR8=%.8X R9=%.8X RA=%.8X RB=%.8X\nRC=%.8X RD=%.8X RE=%.8X RF=%.8X", SH2_Get_R(&M_SH2, 0), SH2_Get_R(&M_SH2, 1), SH2_Get_R(&M_SH2, 2), SH2_Get_R(&M_SH2, 3), SH2_Get_R(&M_SH2, 4), SH2_Get_R(&M_SH2, 5), SH2_Get_R(&M_SH2, 6), SH2_Get_R(&M_SH2, 7), SH2_Get_R(&M_SH2, 8), SH2_Get_R(&M_SH2, 9), SH2_Get_R(&M_SH2, 0xA), SH2_Get_R(&M_SH2, 0xB), SH2_Get_R(&M_SH2, 0xC), SH2_Get_R(&M_SH2, 0xD), SH2_Get_R(&M_SH2, 0xE), SH2_Get_R(&M_SH2, 0xF));
	SendDlgItemMessage(hMSH2, IDC_MSH2_STATUS_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "GBR=%.8X VBR=%.8X PR=%.8X\nMACH=%.8X MACL=%.8X\nIL=%.2X IV=%.2X", SH2_Get_GBR(&M_SH2), SH2_Get_VBR(&M_SH2), SH2_Get_PR(&M_SH2), SH2_Get_MACH(&M_SH2), SH2_Get_MACL(&M_SH2), M_SH2.INT.Prio, M_SH2.INT.Vect);
	SendDlgItemMessage(hMSH2, IDC_MSH2_STATUS_DATA, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", SH2_Get_PC(&M_SH2));//(M_SH2.PC - M_SH2.Base_PC) - 4);
	SendDlgItemMessage(hMSH2, IDC_MSH2_PC, WM_SETTEXT, (WPARAM)0, (LPARAM)debug_string);

}

void Dump32XRom_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_ROM");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 32X ROM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 4 * 1024 * 1024;
	WriteFile(hFr, _32X_Rom, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("32X ROM dumped", 1500);
}

void Dump32XRam_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_MSH2");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 32X memory";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 256 * 1024;
	WriteFile(hFr, _32X_Ram, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("32X RAM dumped", 1500);
}


void DumpMSH2Cache_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_MSH2");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Cache dump (*.dat)\0*.dat\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump MSH2 cache";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "dat";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 0x1000;
	WriteFile(hFr, M_SH2.Cache, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("MSH2 Cache dumped", 1500);
}

void SwitchMSH2ViewMode_KMod()
{
	ShowWindow(GetDlgItem(hMSH2, IDC_MSH2_DASMBOX), MSH2_IsDisasmView() ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hMSH2, IDC_MSH2_HEXBOX), MSH2_IsDisasmView() ? SW_HIDE : SW_SHOW);

	if (MSH2_ViewMode & 2)
	{
		SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_ROM, WM_SETTEXT, 0,
			(LPARAM)((MSH2_ViewMode & 1) ? "View ROM" : "View Disasm"));
	}
	else if (MSH2_ViewMode & 8)
	{
		SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, 0,
			(LPARAM)((MSH2_ViewMode & 4) ? "View RAM" : "View Disasm"));
	}
}

BOOL CALLBACK MSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	unsigned int curPC;
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_JUMP_TO_INPUT, WM_SETTEXT, (WPARAM)0, (LPARAM)"0x00000000");

		SubclassEditMaxText(hwnd, IDC_MSH2_JUMP_TO_INPUT);

		mSH2_reset();
		break;

	case WM_SHOWWINDOW:
		if (wParam)
		{
			SwitchMSH2ViewMode_KMod();
			UpdateMSH2_KMod();
			RestoreMSH2Position();
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MSH2_DASMBOX:
			if (HIWORD(wParam) == DASMBOXN_SCROLL)
			{
				if ((MSH2_ViewMode & 2) && (MSH2_ViewMode & 1))
					MSH2_StartLineROMDisasm = (unsigned int)lParam;
				else if ((MSH2_ViewMode & 8) && (MSH2_ViewMode & 4))
					MSH2_StartLineRAMDisasm = (unsigned int)lParam;
			}
			break;
		case IDC_MSH2_HEXBOX:
			if (HIWORD(wParam) == HEXBOXN_SCROLL)
			{
				if (MSH2_ViewMode & 2)
					MSH2_StartLineROM = (unsigned int)lParam;
				else if (MSH2_ViewMode & 8)
					MSH2_StartLineRAM = (unsigned int)lParam;
				else if (MSH2_ViewMode & 0x20)
					MSH2_StartLineCache = (unsigned int)lParam;
			}
			break;
		case IDC_MSH2_DUMP_ROM:
			Dump32XRom_KMod(hwnd);
			break;
		case IDC_MSH2_DUMP_RAM:
			Dump32XRam_KMod(hwnd);
			break;
		case IDC_MSH2_DUMP_CACHE:
			DumpMSH2Cache_KMod(hwnd);
			break;
		case IDC_MSH2_VIEW_ROM:
			MSH2_ViewMode &= 0x15;
			MSH2_ViewMode ^= 0x1;
			MSH2_ViewMode |= 0x2;
			SwitchMSH2ViewMode_KMod();
			UpdateMSH2_KMod();
			RestoreMSH2Position();
			break;
		case IDC_MSH2_VIEW_RAM:
			MSH2_ViewMode &= 0x15;
			MSH2_ViewMode ^= 0x4;
			MSH2_ViewMode |= 0x8;

			SwitchMSH2ViewMode_KMod();
			UpdateMSH2_KMod();
			RestoreMSH2Position();
			break;
		case IDC_MSH2_VIEW_CACHE:
			MSH2_ViewMode &= 0x15;
			MSH2_ViewMode |= 0x20;

			SwitchMSH2ViewMode_KMod();
			UpdateMSH2_KMod();
			RestoreMSH2Position();
			break;

		case IDC_MSH2_JUMP_TO_INPUT:
			if (HIWORD(wParam) == EN_MAXTEXT)
			{
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_MSH2_JUMP_TO, 0), (LPARAM)GetDlgItem(hwnd, IDC_MSH2_JUMP_TO_INPUT));
			}
			break;
		case IDC_MSH2_JUMP_TO:
		{
			char tmp_string[256];
			GetDlgItemText(hwnd, IDC_MSH2_JUMP_TO_INPUT, tmp_string, 256);
			if (tmp_string[0] == '0' && tmp_string[1] == 'x')
			{
				curPC = strtoul(tmp_string + 2, NULL, 16);
				if (curPC >= 0x02000000 && curPC < 0x02400000)
				{
					MSH2_ViewMode &= 0x15;
					MSH2_ViewMode |= 0x2;
					MSH2_StartLineROM = (curPC - 0x02000000) / 8;
					MSH2_StartLineROMDisasm = curPC;
					SwitchMSH2ViewMode_KMod();
					UpdateMSH2_KMod();
					JumpMSH2To(curPC);
				}
				else if (curPC >= 0x06000000 && curPC < 0x06040000)
				{
					MSH2_ViewMode &= 0x15;
					MSH2_ViewMode |= 0x8;
					MSH2_StartLineRAM = (curPC - 0x06000000) / 8;
					MSH2_StartLineRAMDisasm = curPC;
					SwitchMSH2ViewMode_KMod();
					UpdateMSH2_KMod();
					JumpMSH2To(curPC);
				}
			}
		}
		break;

		case IDC_MSH2_PC:
			curPC = SH2_Get_PC(&M_SH2); //(M_SH2.PC - M_SH2.Base_PC) - 4;
			if (curPC >= 0x02000000 && curPC < 0x02400000)
			{
				MSH2_ViewMode &= 0x15;
				MSH2_ViewMode |= 0x2;
				MSH2_StartLineROM = (curPC - 0x02000000) / 8;
				MSH2_StartLineROMDisasm = curPC;
				SwitchMSH2ViewMode_KMod();
				UpdateMSH2_KMod();
				JumpMSH2To(curPC);
			}
			else if (curPC >= 0x06000000 && curPC < 0x06040000)
			{
				MSH2_ViewMode &= 0x15;
				MSH2_ViewMode |= 0x8;
				MSH2_StartLineRAM = (curPC - 0x06000000) / 8;
				MSH2_StartLineRAMDisasm = curPC;
				SwitchMSH2ViewMode_KMod();
				UpdateMSH2_KMod();
				JumpMSH2To(curPC);
			}
			break;

		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_32_MSH2);
		break;

	case WM_DESTROY:
		DeleteObject((HGDIOBJ)hFont);
		
		mSH2_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void mSH2_create(HINSTANCE hInstance, HWND hWndParent)
{
	hMSH2 = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG32X_MSH2), hWndParent, MSH2DlgProc);
}

void mSH2_show(BOOL visibility)
{
	ShowWindow(hMSH2, visibility ? SW_SHOW : SW_HIDE);
}

void mSH2_update()
{
	if (OpenedWindow_KMod[DMODE_32_MSH2-1] == FALSE)	return;

	UpdateMSH2_KMod();
}

void mSH2_reset()
{
	MSH2_ViewMode = 0x7; //disasm ROM
	MSH2_StartLineROMDisasm = 0x02000000;
	MSH2_StartLineRAMDisasm = 0x06000000;
	MSH2_StartLineROM = MSH2_StartLineRAM = MSH2_StartLineCache = 0;

	SwitchMSH2ViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void mSH2_destroy()
{
	DestroyWindow(hMSH2);
}

