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
#include "sSH2.h"
#include "hexbox.h"
#include "dasmbox.h"

static HWND hSSH2;
static unsigned char SSH2_ViewMode;
static unsigned int  SSH2_StartLineROMDisasm, SSH2_StartLineRAMDisasm, SSH2_StartLineRAM, SSH2_StartLineROM, SSH2_StartLineCache;
static CHAR debug_string[1024];

static BOOL SSH2_IsDisasmView(void)
{
	return ((SSH2_ViewMode & 2) && (SSH2_ViewMode & 1)) ||
		((SSH2_ViewMode & 8) && (SSH2_ViewMode & 4));
}

static BOOL CALLBACK GetSSH2DasmInstruction(void* context, DWORD offset, DWORD* nextOffset, LPSTR text, UINT textCapacity)
{
	char instruction[DASMBOX_TEXT_CAPACITY];
	DWORD startAddress, endAddress, instructionAddress;
	(void)context;

	if (SSH2_ViewMode & 2)
	{
		startAddress = 0x02000000;
		endAddress = 0x02400000;
	}
	else
	{
		startAddress = 0x06000000;
		endAddress = 0x06040000;
	}
	if (offset > endAddress - startAddress - 2)
		return FALSE;

	instructionAddress = startAddress + offset;
	SH2Disasm(instruction, instructionAddress, SH2_Read_Word(&S_SH2, instructionAddress), 0);
	instruction[DASMBOX_TEXT_CAPACITY - 1] = 0;
	*nextOffset = offset + 2;
	lstrcpynA(text, instruction + 8, textCapacity);
	return TRUE;
}

static void JumpSSH2To(DWORD address)
{
	if (SSH2_IsDisasmView())
		SendDlgItemMessage(hSSH2, IDC_SSH2_DASMBOX, DASMBOX_GOTO_ADDRESS, (WPARAM)address, 0);
	else
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_GOTO_ADDRESS, (WPARAM)address, 0);
}

static void RestoreSSH2Position(void)
{
	if ((SSH2_ViewMode & 2) && (SSH2_ViewMode & 1))
		JumpSSH2To(SSH2_StartLineROMDisasm);
	else if ((SSH2_ViewMode & 8) && (SSH2_ViewMode & 4))
		JumpSSH2To(SSH2_StartLineRAMDisasm);
	else if (SSH2_ViewMode & 2)
		JumpSSH2To(0x02000000 + SSH2_StartLineROM * 8);
	else if (SSH2_ViewMode & 8)
		JumpSSH2To(0x06000000 + SSH2_StartLineRAM * 8);
	else if (SSH2_ViewMode & 0x20)
		JumpSSH2To(0xC0000000 + SSH2_StartLineCache * 8);
}

void UpdateSSH2_KMod()
{
	DASMBOX_SOURCE dasmSource;

	if ((SSH2_ViewMode & 2) && (SSH2_ViewMode & 1))
	{
		dasmSource.size = sizeof(_32X_Rom);
		dasmSource.addressLength = 8;
		dasmSource.startAddress = 0x02000000;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetSSH2DasmInstruction;
		SendDlgItemMessage(hSSH2, IDC_SSH2_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if ((SSH2_ViewMode & 8) && (SSH2_ViewMode & 4))
	{
		dasmSource.size = sizeof(_32X_Ram);
		dasmSource.addressLength = 8;
		dasmSource.startAddress = 0x06000000;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetSSH2DasmInstruction;
		SendDlgItemMessage(hSSH2, IDC_SSH2_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if (SSH2_ViewMode & 2)
	{
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(_32X_Rom), (LPARAM)_32X_Rom);
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 4, HEXBOX_MODE_WORD), (LPARAM)0x02000000);
	}
	else if (SSH2_ViewMode & 8)
	{
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(_32X_Ram), (LPARAM)_32X_Ram);
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 4, HEXBOX_MODE_WORD), (LPARAM)0x06000000);
	}
	else if (SSH2_ViewMode & 0x20)
	{
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_DATA, sizeof(S_SH2.Cache), (LPARAM)S_SH2.Cache);
		SendDlgItemMessage(hSSH2, IDC_SSH2_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(8, 4, HEXBOX_MODE_WORD), (LPARAM)0xC0000000);
	}
	wsprintf(debug_string, "T=%d S=%d Q=%d M=%d I=%.1X SR=%.4X Status=%.4X", SH2_Get_SR(&S_SH2) & 1, (SH2_Get_SR(&S_SH2) >> 1) & 1, (SH2_Get_SR(&S_SH2) >> 8) & 1, (SH2_Get_SR(&S_SH2) >> 9) & 1, (SH2_Get_SR(&S_SH2) >> 4) & 0xF, SH2_Get_SR(&S_SH2), S_SH2.Status & 0xFFFF);
	SendDlgItemMessage(hSSH2, IDC_SSH2_STATUS_SR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "R0=%.8X R1=%.8X R2=%.8X R3=%.8X\nR4=%.8X R5=%.8X R6=%.8X R7=%.8X\nR8=%.8X R9=%.8X RA=%.8X RB=%.8X\nRC=%.8X RD=%.8X RE=%.8X RF=%.8X", SH2_Get_R(&S_SH2, 0), SH2_Get_R(&S_SH2, 1), SH2_Get_R(&S_SH2, 2), SH2_Get_R(&S_SH2, 3), SH2_Get_R(&S_SH2, 4), SH2_Get_R(&S_SH2, 5), SH2_Get_R(&S_SH2, 6), SH2_Get_R(&S_SH2, 7), SH2_Get_R(&S_SH2, 8), SH2_Get_R(&S_SH2, 9), SH2_Get_R(&S_SH2, 0xA), SH2_Get_R(&S_SH2, 0xB), SH2_Get_R(&S_SH2, 0xC), SH2_Get_R(&S_SH2, 0xD), SH2_Get_R(&S_SH2, 0xE), SH2_Get_R(&S_SH2, 0xF));
	SendDlgItemMessage(hSSH2, IDC_SSH2_STATUS_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "GBR=%.8X VBR=%.8X PR=%.8X\nMACH=%.8X MACL=%.8X\nIL=%.2X IV=%.2X", SH2_Get_GBR(&S_SH2), SH2_Get_VBR(&S_SH2), SH2_Get_PR(&S_SH2), SH2_Get_MACH(&S_SH2), SH2_Get_MACL(&S_SH2), S_SH2.INT.Prio, S_SH2.INT.Vect);
	SendDlgItemMessage(hSSH2, IDC_SSH2_STATUS_DATA, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", SH2_Get_PC(&S_SH2));//(S_SH2.PC - S_SH2.Base_PC) - 4);
	SendDlgItemMessage(hSSH2, IDC_SSH2_PC, WM_SETTEXT, (WPARAM)0, (LPARAM)debug_string);

}

void DumpSSH2Cache_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_SSH2");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Cache dump (*.dat)\0*.dat\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump SSH2 cache";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "dat";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 0x1000;
	WriteFile(hFr, S_SH2.Cache, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("SSH2 Cache dumped", 1500);
}

void SwitchSSH2ViewMode_KMod()
{
	BOOL editableRamView = (SSH2_ViewMode & 8) && !(SSH2_ViewMode & 4);
	HWND hexBox = GetDlgItem(hSSH2, IDC_SSH2_HEXBOX);
	LONG_PTR hexBoxStyle = GetWindowLongPtr(hexBox, GWL_STYLE);
	if (editableRamView)
		hexBoxStyle |= HEXBOX_STYLE_EDITABLE;
	else
		hexBoxStyle &= ~((LONG_PTR)HEXBOX_STYLE_EDITABLE);
	SetWindowLongPtr(hexBox, GWL_STYLE, hexBoxStyle);

	ShowWindow(GetDlgItem(hSSH2, IDC_SSH2_DASMBOX), SSH2_IsDisasmView() ? SW_SHOW : SW_HIDE);
	ShowWindow(hexBox, SSH2_IsDisasmView() ? SW_HIDE : SW_SHOW);

	if (SSH2_ViewMode & 2)
	{
		SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_ROM, WM_SETTEXT, 0,
			(LPARAM)((SSH2_ViewMode & 1) ? "View ROM" : "View Disasm"));
	}
	else if (SSH2_ViewMode & 8)
	{
		SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, 0,
			(LPARAM)((SSH2_ViewMode & 4) ? "View RAM" : "View Disasm"));
	}
}

BOOL CALLBACK SSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	unsigned int curPC;
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_JUMP_TO_INPUT, WM_SETTEXT, (WPARAM)0, (LPARAM)"0x00000000");

		SubclassEditMaxText(hwnd, IDC_SSH2_JUMP_TO_INPUT);

		sSH2_reset();
		
		break;

	case WM_SHOWWINDOW:
		if (wParam)
		{
			SwitchSSH2ViewMode_KMod();
			UpdateSSH2_KMod();
			RestoreSSH2Position();
		}
		break;

	case WM_NOTIFY:
		if (lParam)
		{
			HEXBOX_EDIT_NOTIFICATION* notification = (HEXBOX_EDIT_NOTIFICATION*)lParam;
			if (notification->hdr.idFrom == IDC_SSH2_HEXBOX &&
				notification->hdr.code == HEXBOXN_EDIT &&
				(SSH2_ViewMode & 8) && !(SSH2_ViewMode & 4) &&
				notification->itemSize == 2 &&
				notification->offset < sizeof(_32X_Ram) &&
				sizeof(_32X_Ram) - notification->offset >= 2)
			{
				_32X_Ram[notification->offset] = (unsigned char)(notification->value >> 8);
				_32X_Ram[notification->offset + 1] = (unsigned char)notification->value;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SSH2_DASMBOX:
			if (HIWORD(wParam) == DASMBOXN_SCROLL)
			{
				if ((SSH2_ViewMode & 2) && (SSH2_ViewMode & 1))
					SSH2_StartLineROMDisasm = (unsigned int)lParam;
				else if ((SSH2_ViewMode & 8) && (SSH2_ViewMode & 4))
					SSH2_StartLineRAMDisasm = (unsigned int)lParam;
			}
			break;
		case IDC_SSH2_HEXBOX:
			if (HIWORD(wParam) == HEXBOXN_SCROLL)
			{
				if (SSH2_ViewMode & 2)
					SSH2_StartLineROM = (unsigned int)lParam;
				else if (SSH2_ViewMode & 8)
					SSH2_StartLineRAM = (unsigned int)lParam;
				else if (SSH2_ViewMode & 0x20)
					SSH2_StartLineCache = (unsigned int)lParam;
			}
			break;
		case IDC_SSH2_DUMP_ROM:
			//Dump32XRom_KMod( hwnd );
			break;
		case IDC_SSH2_DUMP_RAM:
			//Dump32XRam_KMod( hwnd );
			break;
		case IDC_SSH2_DUMP_CACHE:
			DumpSSH2Cache_KMod(hwnd);
			break;
		case IDC_SSH2_VIEW_ROM:
			SSH2_ViewMode &= 0x15;
			SSH2_ViewMode ^= 0x1;
			SSH2_ViewMode |= 0x2;
			SwitchSSH2ViewMode_KMod();
			UpdateSSH2_KMod();
			RestoreSSH2Position();
			break;
		case IDC_SSH2_VIEW_RAM:
			SSH2_ViewMode &= 0x15;
			SSH2_ViewMode ^= 0x4;
			SSH2_ViewMode |= 0x8;

			SwitchSSH2ViewMode_KMod();
			UpdateSSH2_KMod();
			RestoreSSH2Position();
			break;
		case IDC_SSH2_VIEW_CACHE:
			SSH2_ViewMode &= 0x15;
			SSH2_ViewMode |= 0x20;

			SwitchSSH2ViewMode_KMod();
			UpdateSSH2_KMod();
			RestoreSSH2Position();
			break;

		case IDC_SSH2_JUMP_TO_INPUT:
			if (HIWORD(wParam) == EN_MAXTEXT)
			{
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SSH2_JUMP_TO, 0), (LPARAM)GetDlgItem(hwnd, IDC_SSH2_JUMP_TO_INPUT));
			}
			break;

		case IDC_SSH2_JUMP_TO:
		{
			char jump_input[32];
			unsigned int jump_address;
			GetDlgItemText(hwnd, IDC_SSH2_JUMP_TO_INPUT, jump_input, sizeof(jump_input));
			if (sscanf(jump_input, "%x", &jump_address) == 1)
			{
				if (jump_address >= 0x02000000 && jump_address < 0x02400000)
				{
					SSH2_ViewMode &= 0x15;
					SSH2_ViewMode |= 0x2;
					SSH2_StartLineROM = (jump_address - 0x02000000) / 8;
					SSH2_StartLineROMDisasm = jump_address - 0x02000000	;
					SwitchSSH2ViewMode_KMod();
					UpdateSSH2_KMod();
					JumpSSH2To(jump_address);
				}
				else if (jump_address >= 0x06000000 && jump_address < 0x06040000)
				{
					SSH2_ViewMode &= 0x15;
					SSH2_ViewMode |= 0x8;
					SSH2_StartLineRAM = (jump_address - 0x06000000) / 8;
					SSH2_StartLineRAMDisasm = jump_address - 0x06000000;
					SwitchSSH2ViewMode_KMod();
					UpdateSSH2_KMod();
					JumpSSH2To(jump_address);
				}
			}
			break;
		}
		case IDC_SSH2_PC:
			curPC = SH2_Get_PC(&S_SH2); //(S_SH2.PC - S_SH2.Base_PC) - 4;
			if (curPC >= 0x02000000 && curPC < 0x02400000)
			{
				SSH2_ViewMode &= 0x15;
				SSH2_ViewMode |= 0x2;
				SSH2_StartLineROM = (curPC - 0x02000000) / 8;
				SSH2_StartLineROMDisasm = (curPC - 0x02000000);
				SwitchSSH2ViewMode_KMod();
				UpdateSSH2_KMod();
				JumpSSH2To(curPC);
			}
			else if (curPC >= 0x06000000 && curPC < 0x06040000)
			{
				SSH2_ViewMode &= 0x15;
				SSH2_ViewMode |= 0x8;
				SSH2_StartLineRAM = (curPC - 0x06000000) / 8;
				SSH2_StartLineRAMDisasm = (curPC - 0x06000000);
				SwitchSSH2ViewMode_KMod();
				UpdateSSH2_KMod();
				JumpSSH2To(curPC);
			}
			break;

		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_32_SSH2);
		break;

	case WM_DESTROY:
		DeleteObject((HGDIOBJ)hFont);
		sSH2_destroy( );
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void sSH2_create(HINSTANCE hInstance, HWND hWndParent)
{
	hSSH2 = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG32X_SSH2), hWndParent, SSH2DlgProc);
}

void sSH2_show(BOOL visibility)
{
	ShowWindow(hSSH2, visibility ? SW_SHOW : SW_HIDE);
}

void sSH2_update()
{
	if (OpenedWindow_KMod[DMODE_32_SSH2-1] == FALSE)	return;

	UpdateSSH2_KMod();
}

void sSH2_reset()
{
	SSH2_ViewMode = 0x7; //disasm ROM
	SSH2_StartLineROMDisasm = 0x0;
	SSH2_StartLineRAMDisasm = 0x0;
	SSH2_StartLineROM = SSH2_StartLineRAM = SSH2_StartLineCache = 0;
	SwitchSSH2ViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void sSH2_destroy()
{
	DestroyWindow(hSSH2);
}

