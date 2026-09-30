#include <windows.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"

#include "../mem_Z80.h"
#include "../z80.h"
#include "../z80dis.h"
#include "../Mem_M68k.h" //for Z80_State
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "z80.h"
#include "hexbox.h"
#include "dasmbox.h"

static HWND hZ80;
static BOOL Z80_ViewMode;
static unsigned int  Z80_StartLineDisasm, Z80_StartLineMem;
static CHAR debug_string[1024];
HFONT hFont;

static BOOL CALLBACK GetZ80DasmInstruction(void* context, DWORD address, DWORD* nextAddress, LPSTR text, UINT textCapacity)
{
	char instruction[128];
	int pc;
	int textLength;
	(void)context;

	if (address >= sizeof(Ram_Z80))
		return FALSE;

	pc = (int)address;
	z80dis(Ram_Z80, &pc, instruction);

	*nextAddress = (DWORD)pc;
	textLength = lstrlenA(instruction);
	if (textLength && instruction[textLength - 1] == '\n')
		instruction[textLength - 1] = 0;
	lstrcpynA(text, instruction + 6, textCapacity);
	return TRUE;
}

void SwitchZ80ViewMode_KMod()
{
	ShowWindow(GetDlgItem(hZ80, IDC_Z80_DASMBOX), Z80_ViewMode ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hZ80, IDC_Z80_HEXBOX), Z80_ViewMode ? SW_SHOW : SW_HIDE);
	SendDlgItemMessage(hZ80, IDC_Z80_VIEW_MEM, WM_SETTEXT, 0,
		(LPARAM)(Z80_ViewMode ? "View Disasm" : "View Memory"));
}

void UpdateZ80_KMod()
{
	if (Z80_ViewMode == 0)
	{
		DASMBOX_SOURCE dasmSource;
		dasmSource.lineCount = sizeof(Ram_Z80);
		dasmSource.addressLength = 4;
		dasmSource.startAddress = 0;
		dasmSource.context = NULL;
		dasmSource.getInstruction = GetZ80DasmInstruction;
		SendDlgItemMessage(hZ80, IDC_Z80_DASMBOX, DASMBOX_SET_SOURCE, 0, (LPARAM)&dasmSource);
	}
	else
	{
		SendDlgItemMessage(hZ80, IDC_Z80_HEXBOX, HEXBOX_SET_DATA, sizeof(Ram_Z80), (LPARAM)Ram_Z80);
		SendDlgItemMessage(hZ80, IDC_Z80_HEXBOX, HEXBOX_SET_LAYOUT,
			HEXBOX_LAYOUT_WPARAM(4, 8, HEXBOX_MODE_BYTE), 0);
	}


	wsprintf(debug_string, "AF =%.4X BC =%.4X DE =%.4X HL =%.4X\nAF2=%.4X BC2=%.4X DE2=%.4X HL2=%.4X", z80_Get_AF(&M_Z80), M_Z80.BC.w.BC, M_Z80.DE.w.DE, M_Z80.HL.w.HL, z80_Get_AF2(&M_Z80), M_Z80.BC2.w.BC2, M_Z80.DE2.w.DE2, M_Z80.HL2.w.HL2);
	SendDlgItemMessage(hZ80, IDC_Z80_STATUS_RS1, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "IX =%.4X IY =%.4X SP =%.4X ", M_Z80.IX.w.IX, M_Z80.IY.w.IY, M_Z80.SP.w.SP);
	SendDlgItemMessage(hZ80, IDC_Z80_STATUS_RS3, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "IFF1=%d IFF2=%d I=%.2X R=%.2X IM=%.2X", M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I, M_Z80.R.b.R1, M_Z80.IM);
	SendDlgItemMessage(hZ80, IDC_Z80_STATUS_MISC, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "S=%d Z=%d Y=%d H=%d X=%d P=%d N=%d C=%d", (z80_Get_AF(&M_Z80) & 0x80) >> 7, (z80_Get_AF(&M_Z80) & 0x40) >> 6, (z80_Get_AF(&M_Z80) & 0x20) >> 5, (z80_Get_AF(&M_Z80) & 0x10) >> 4, (z80_Get_AF(&M_Z80) & 0x08) >> 3, (z80_Get_AF(&M_Z80) & 0x04) >> 2, (z80_Get_AF(&M_Z80) & 0x02) >> 1, (z80_Get_AF(&M_Z80) & 0x01) >> 0);
	SendDlgItemMessage(hZ80, IDC_Z80_STATUS_FLAG, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "Bank = %0.8X State=%.2X", Bank_Z80, Z80_State);
	SendDlgItemMessage(hZ80, IDC_Z80_STATUS_BANK, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.4X", z80_Get_PC(&M_Z80));
	SendDlgItemMessage(hZ80, IDC_Z80_PC, WM_SETTEXT, 0, (LPARAM)debug_string);

	/*
	sprintf(GString, "Status=%.2X ILine=%.2X IVect=%.2X\n", M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	sprintf(GString, "Bank68K=%.8X State=%.2X\n", M_Z80.Status & 0xFF, Bank_M68K, Z80_State);
	*/

}

static void JumpZ80To(DWORD address)
{
	if (Z80_ViewMode == 0)
		SendDlgItemMessage(hZ80, IDC_Z80_DASMBOX, DASMBOX_GOTO_ADDRESS, (WPARAM)address, 0);
	else
		SendDlgItemMessage(hZ80, IDC_Z80_HEXBOX, HEXBOX_GOTO_ADDRESS, (WPARAM)address, 0);
}

static void RestoreZ80Position(void)
{
	JumpZ80To(Z80_ViewMode ? Z80_StartLineMem * 8 : Z80_StartLineDisasm);
}

void DumpZ80_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_Z80");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump Z80 memory";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 8 * 1024;
	WriteFile(hFr, Ram_Z80, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("Z80 Ram dumped", 1500);
}

BOOL CALLBACK Z80DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS1, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS2, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_RS3, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_MISC, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_FLAG, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_STATUS_BANK, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_Z80_JUMP_TO_INPUT, WM_SETTEXT, (WPARAM)0, (LPARAM)"0x0000");

		SubclassEditMaxText(hwnd, IDC_Z80_JUMP_TO_INPUT);

		z80debug_reset();
		break;

	case WM_SHOWWINDOW:
		if (wParam)
		{
			SwitchZ80ViewMode_KMod();
			UpdateZ80_KMod();
			RestoreZ80Position();
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_Z80_DASMBOX:
			if (HIWORD(wParam) == DASMBOXN_SCROLL)
				Z80_StartLineDisasm = (unsigned int)lParam;
			break;
		case IDC_Z80_HEXBOX:
			if (HIWORD(wParam) == HEXBOXN_SCROLL)
				Z80_StartLineMem = (unsigned int)lParam;
			break;
		case IDC_Z80_DUMP_MEM:
			DumpZ80_KMod(hwnd);
			break;

		case IDC_Z80_VIEW_MEM:
			Z80_ViewMode = !Z80_ViewMode;
			SwitchZ80ViewMode_KMod();
			UpdateZ80_KMod();
			RestoreZ80Position();
			break;
		case IDC_Z80_JUMP_TO_INPUT:

			if (HIWORD(wParam) == EN_MAXTEXT)
			{
				// Run your Jump handling logic here!
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_Z80_JUMP_TO, 0), (LPARAM)GetDlgItem(hwnd, IDC_Z80_JUMP_TO_INPUT));
			}
			break;

		case IDC_Z80_JUMP_TO:
		{
			DWORD adr;
			char tmp_string[32];
			GetDlgItemText(hwnd, IDC_Z80_JUMP_TO_INPUT, tmp_string, 32);
			adr = strtoul(tmp_string, NULL, 16);
			if (Z80_ViewMode == 0)
			{
				Z80_StartLineDisasm = adr;
				UpdateZ80_KMod();
				JumpZ80To(adr);
			}
			else if (Z80_ViewMode == 1)
			{
				Z80_StartLineMem = adr / 8;
				UpdateZ80_KMod();
				JumpZ80To(adr);
			}
			break;
		}
		case IDC_Z80_PC:
			if (Z80_ViewMode == 0)
			{
				Z80_StartLineDisasm = z80_Get_PC(&M_Z80);
				UpdateZ80_KMod();
				JumpZ80To(Z80_StartLineDisasm);
			}
			break;
		}

		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_Z80);
		break;

	case WM_DESTROY:
		z80debug_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void z80debug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hZ80 = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGZ80), hWndParent, Z80DlgProc);
}

void z80debug_show(BOOL visibility)
{
	ShowWindow(hZ80, visibility ? SW_SHOW : SW_HIDE);
}

void z80debug_update()
{
	if (OpenedWindow_KMod[DMODE_Z80-1] == FALSE)	return;

	UpdateZ80_KMod();
}

void z80debug_reset()
{
	Z80_ViewMode = 0; //disasm
	Z80_StartLineDisasm = Z80_StartLineMem = 0;
	SwitchZ80ViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void z80debug_destroy()
{
	DeleteObject((HGDIOBJ)hFont);
	DestroyWindow(hZ80);
}

void z80debug_dump()
{
	DumpZ80_KMod(hZ80);
}