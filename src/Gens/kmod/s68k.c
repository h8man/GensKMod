#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../M68KD.h"
#include "../gens.h"
#include "../resource.h"
#include "../Star_68k.h"
#include "../Mem_M68k.h"
#include "../Mem_S68k.h"
#include "../Misc.h" //for Byte_Swap
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "s68k.h"
#include "hexbox.h"
#include "dasmbox.h"

static HWND hCD_68K;
static int Current_PC_S68K;
static CHAR debug_string[1024];

unsigned short Next_Word_S68K(void)
{
	unsigned short val;

	val = S68K_RW(Current_PC_S68K);

	Current_PC_S68K += 2;

	return(val);
}


unsigned int Next_Long_S68K(void)
{
	unsigned int val;

	val = S68K_RW(Current_PC_S68K);
	val <<= 16;
	val |= S68K_RW(Current_PC_S68K + 2);

	Current_PC_S68K += 4;

	return(val);
}


unsigned char S68k_ViewMode;
unsigned int  S68k_StartLineDisasm, S68k_StartLineWRAM, S68k_StartLinePRAM;

static BOOL CALLBACK GetS68kDasmInstruction(void* context, DWORD offset, DWORD* nextOffset, LPSTR text, UINT textCapacity)
{
	char* instruction;
	(void)context;

	Current_PC_S68K = (int)offset;
	instruction = M68KDisasm(Next_Word_S68K, Next_Long_S68K);
	*nextOffset = Current_PC_S68K;
	lstrcpynA(text, instruction ? instruction : "", textCapacity);
	return TRUE;
}



void SwitchS68kViewMode_KMod()
{
	BOOL disasmView = (S68k_ViewMode & 2) && (S68k_ViewMode & 1);
	ShowWindow(GetDlgItem(hCD_68K, IDC_S68K_DASMBOX), disasmView ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hCD_68K, IDC_S68K_HEXBOX), disasmView ? SW_HIDE : SW_SHOW);
	SendDlgItemMessage(hCD_68K, IDC_S68K_VIEW_PRAM, WM_SETTEXT, 0,
		(LPARAM)(disasmView ? "View PRAM" : "View Disasm"));
}


void UpdateCD_68K_KMod()
{
	DASMBOX_SOURCE dasmSource;

	if ((S68k_ViewMode & 2) && (S68k_ViewMode & 1))
	{
		dasmSource.size = Rom_Size;
		dasmSource.addressLength = 5;
		dasmSource.startAddress = 0;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetS68kDasmInstruction;
		SendDlgItemMessage(hCD_68K, IDC_S68K_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else if (S68k_ViewMode & 2)
	{
		SendDlgItemMessage(hCD_68K, IDC_S68K_HEXBOX, HEXBOX_SET_DATA, sizeof(Ram_Prg), (LPARAM)Ram_Prg);
		SendDlgItemMessage(hCD_68K, IDC_S68K_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(5, 4, HEXBOX_MODE_WORD | HEXBOX_ENDIAN_LITTLE), 0);
	}
	else if (S68k_ViewMode & 8)
	{
		SendDlgItemMessage(hCD_68K, IDC_S68K_HEXBOX, HEXBOX_SET_DATA, sizeof(Ram_Word_1M), (LPARAM)Ram_Word_1M);
		SendDlgItemMessage(hCD_68K, IDC_S68K_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(5, 4, HEXBOX_MODE_WORD | HEXBOX_ENDIAN_LITTLE), 0);
	}


	wsprintf(debug_string, "X=%d N=%d Z=%d V=%d C=%d  SR=%.4X Cycles=%.10d", (sub68k_context.sr & 0x10) ? 1 : 0, (sub68k_context.sr & 0x8) ? 1 : 0, (sub68k_context.sr & 0x4) ? 1 : 0, (sub68k_context.sr & 0x2) ? 1 : 0, (sub68k_context.sr & 0x1) ? 1 : 0, sub68k_context.sr, sub68k_context.odometer);
	SendDlgItemMessage(hCD_68K, IDC_S68K_STATUS_SR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "A0=%.8X A1=%.8X A2=%.8X A3=%.8X A4=%.8X A5=%.8X A6=%.8X A7=%.8X\n", sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.areg[2], sub68k_context.areg[3], sub68k_context.areg[4], sub68k_context.areg[5], sub68k_context.areg[6], sub68k_context.areg[7]);
	SendDlgItemMessage(hCD_68K, IDC_S68K_STATUS_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "D0=%.8X D1=%.8X D2=%.8X D3=%.8X D4=%.8X D5=%.8X D6=%.8X D7=%.8X\n", sub68k_context.dreg[0], sub68k_context.dreg[1], sub68k_context.dreg[2], sub68k_context.dreg[3], sub68k_context.dreg[4], sub68k_context.dreg[5], sub68k_context.dreg[6], sub68k_context.dreg[7]);
	SendDlgItemMessage(hCD_68K, IDC_S68K_STATUS_DATA, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", sub68k_context.pc);
	SendDlgItemMessage(hCD_68K, IDC_S68K_PC, WM_SETTEXT, (WPARAM)0, (LPARAM)debug_string);

}

static void JumpS68kTo(DWORD address)
{
	if ((S68k_ViewMode & 2) && (S68k_ViewMode & 1))
		SendDlgItemMessage(hCD_68K, IDC_S68K_DASMBOX, DASMBOX_GOTO_ADDRESS, (WPARAM)address, 0);
	else
		SendDlgItemMessage(hCD_68K, IDC_S68K_HEXBOX, HEXBOX_GOTO_ADDRESS, (WPARAM)address, 0);
}

static void RestoreS68kAddressLine(void)
{
	if ((S68k_ViewMode & 2) && (S68k_ViewMode & 1))
		JumpS68kTo(S68k_StartLineDisasm);
	else if (S68k_ViewMode & 2)
		JumpS68kTo(S68k_StartLinePRAM * 8);
	else if (S68k_ViewMode & 8)
		JumpS68kTo(S68k_StartLineWRAM * 8);
}



void DumpS68KPRam_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_S68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump CD ROM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;


	dwBytesToWrite = 512 * 1024;
	Byte_Swap(Ram_Prg, dwBytesToWrite);
	WriteFile(hFr, Ram_Prg, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Ram_Prg, dwBytesToWrite);

	CloseHandle(hFr);

	Put_Info("CD ROM dumped", 1500);
}

void DumpS68KWRam_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_S68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump CD 68K memory";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 256 * 1024;
	Byte_Swap(Ram_Word_1M, dwBytesToWrite);
	WriteFile(hFr, Ram_Word_1M, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Ram_Word_1M, dwBytesToWrite);

	CloseHandle(hFr);

	Put_Info("CD 68K RAM dumped", 1500);
}


BOOL CALLBACK CD_68KDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	HFONT hFont = NULL;

	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_S68K_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_S68K_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_S68K_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_S68K_JUMP_TO_INPUT, WM_SETTEXT, (WPARAM)0, (LPARAM)"0x00000000");

		SubclassEditMaxText(hwnd, IDC_S68K_JUMP_TO_INPUT);

		s68kdebug_reset();
		break;

	case WM_SHOWWINDOW:
		SwitchS68kViewMode_KMod();
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_S68K_DASMBOX:
			if (HIWORD(wParam) == DASMBOXN_SCROLL)
				S68k_StartLineDisasm = (unsigned int)lParam;
			break;
		case IDC_S68K_HEXBOX:
			if (HIWORD(wParam) == HEXBOXN_SCROLL)
			{
				if (S68k_ViewMode & 2)
					S68k_StartLinePRAM = (unsigned int)lParam;
				else if (S68k_ViewMode & 8)
					S68k_StartLineWRAM = (unsigned int)lParam;
			}
			break;
		case IDC_S68K_DUMP_PRAM:
			DumpS68KPRam_KMod(hwnd);
			break;

		case IDC_S68K_DUMP_WRAM:
			DumpS68KWRam_KMod(hwnd);
			break;

		case IDC_S68K_VIEW_PRAM:
			S68k_ViewMode &= 0x15;
			S68k_ViewMode ^= 0x1;
			S68k_ViewMode |= 0x2;
			SwitchS68kViewMode_KMod();
			UpdateCD_68K_KMod();
			RestoreS68kAddressLine();
			break;

		case IDC_S68K_VIEW_WRAM:
			S68k_ViewMode &= 0x15;
			S68k_ViewMode ^= 0x4;
			S68k_ViewMode |= 0x8;
			SwitchS68kViewMode_KMod();
			UpdateCD_68K_KMod();
			RestoreS68kAddressLine();
			break;
		
		case IDC_S68K_JUMP_TO_INPUT:
			if (HIWORD(wParam) == EN_MAXTEXT)
			{
				// User pressed Enter in the edit control
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_S68K_JUMP_TO, 0), (LPARAM)GetDlgItem(hwnd, IDC_S68K_JUMP_TO_INPUT));
			}
			break;
		case IDC_S68K_JUMP_TO:
		{
			DWORD adr;
			char tmp_string[32];
			GetDlgItemText(hwnd, IDC_S68K_JUMP_TO_INPUT, tmp_string, 32);
			adr = strtoul(tmp_string, NULL, 16);
			if (S68k_ViewMode & 2)
			{
				S68k_StartLineDisasm = adr;
				S68k_StartLinePRAM = S68k_StartLineDisasm / 8;
				UpdateCD_68K_KMod();
				JumpS68kTo(adr);
			}
			else if (S68k_ViewMode & 8)
			{
				S68k_StartLineWRAM = adr / 8;
				UpdateCD_68K_KMod();
				JumpS68kTo(adr);
			}
			break;
		}

		case IDC_S68K_PC:
			if (S68k_ViewMode & 2)
			{
				S68k_ViewMode &= 0x15;
				S68k_ViewMode |= 0x2;

				S68k_StartLineDisasm = sub68k_context.pc;
				S68k_StartLinePRAM = S68k_StartLineDisasm / 8;
				SwitchS68kViewMode_KMod();
				UpdateCD_68K_KMod();
				JumpS68kTo(sub68k_context.pc);
			}
			break;

		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_CD_68K);
		break;

	case WM_DESTROY:
		DeleteObject((HGDIOBJ)hFont);
		DestroyWindow(hCD_68K);
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void s68kdebug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hCD_68K = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGCD_68K), hWndParent, CD_68KDlgProc);
}

void s68kdebug_show(BOOL visibility)
{
	ShowWindow(hCD_68K, visibility ? SW_SHOW : SW_HIDE);
}

void s68kdebug_update()
{
	if (OpenedWindow_KMod[DMODE_CD_68K-1] == FALSE)	return;

	UpdateCD_68K_KMod();
}

void s68kdebug_reset()
{
	S68k_ViewMode = 2; //disasm
	S68k_StartLineDisasm = S68k_StartLineWRAM = S68k_StartLinePRAM = 0;
	SwitchS68kViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void s68kdebug_destroy()
{
	
}

void s68kdebug_dump()
{
	DumpS68KPRam_KMod(hCD_68K);
}