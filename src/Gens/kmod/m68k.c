#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../M68KD.h"
#include "../gens.h"
#include "../resource.h"
#include "../Star_68k.h"
#include "../Mem_M68k.h"
#include "../Misc.h" //for Byte_Swap
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "m68k.h"

static HWND hM68K;
static int Current_PC_M68K;
static CHAR debug_string[1024];
HFONT hFont;

static unsigned short Next_Word_M68K(void)
{
	unsigned short val;

	val = M68K_RW(Current_PC_M68K);

	Current_PC_M68K += 2;

	return(val);
}


static unsigned int Next_Long_M68K(void)
{
	unsigned int val;

	val = M68K_RW(Current_PC_M68K);
	val <<= 16;
	val |= M68K_RW(Current_PC_M68K + 2);

	Current_PC_M68K += 4;

	return(val);
}


static unsigned char M68_ViewMode;
static unsigned int  M68k_StartLineDisasm, M68k_StartLineRAM, M68k_StartLineROM;



static void SwitchM68kViewMode_KMod()
{
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nPage = 10;

	if (M68_ViewMode == 0)
	{
		// DISASM VIEW
		si.nPos = M68k_StartLineDisasm;
		si.nMax = Rom_Size - 1; // 0x400000 - 28;
	}
	else if (M68_ViewMode == 1)
	{
		// ROM VIEW
		si.nPos = M68k_StartLineROM;
		si.nMax = (Rom_Size / 8) - 1; //(0x400000 >> 3) -1;
	}
	else
	{
		// RAM VIEW
		si.nPos = M68k_StartLineRAM;
		si.nMax = (0x10000 / 8) - 1;
	}
	SetScrollInfo(GetDlgItem(hM68K, IDC_68K_SCROLL), SB_CTL, &si, TRUE);
}



static void UpdateM68k_KMod()
{
	unsigned int i, PC;
	unsigned char tmp_string[256];

	


	SendDlgItemMessage(hM68K, IDC_68K_DISAM, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (M68_ViewMode == 0)
	{
		M68k_StartLineDisasm = GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL), SB_CTL);
		Current_PC_M68K = M68k_StartLineDisasm; //main68k_context.pc;
		for (i = 0; i < 13; i++)
		{
			PC = Current_PC_M68K;
			if (PC >= Rom_Size)
			{
				wsprintf(debug_string, "%.6X", PC);
				Current_PC_M68K += 2;
			}
			else
			{
				wsprintf(debug_string, "%.6X    %-33s", PC, M68KDisasm(Next_Word_M68K, Next_Long_M68K));
			}
			SendDlgItemMessage(hM68K, IDC_68K_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
	}
	else if (M68_ViewMode == 1)
	{
		Byte_Swap(Rom_Data, Rom_Size);
		M68k_StartLineROM = GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL), SB_CTL);
		for (i = 0; i < 13; i++)
		{
			if ((M68k_StartLineROM * 8 + i * 8) > Rom_Size)
			{
				wsprintf(debug_string, "%.6X ", M68k_StartLineROM * 8 + i * 8);
			}
			else
			{
				wsprintf(tmp_string, "%.6X ", M68k_StartLineROM * 8 + i * 8);
				tmp_string[6] = 0x20;
				Hexview((unsigned char *)(Rom_Data + M68k_StartLineROM * 8 + i * 8), tmp_string + 7);
				tmp_string[24] = 0x20;
				Ansiview((unsigned char *)(Rom_Data + M68k_StartLineROM * 8 + i * 8), tmp_string + 25);
				wsprintf(debug_string, "%s", tmp_string);
			}
			SendDlgItemMessage(hM68K, IDC_68K_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
		Byte_Swap(Rom_Data, Rom_Size);
	}
	else if (M68_ViewMode == 2)
	{
		Byte_Swap(Ram_68k, Rom_Size);
		M68k_StartLineRAM = GetScrollPos(GetDlgItem(hM68K, IDC_68K_SCROLL), SB_CTL);
		for (i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "FF%.4X ", M68k_StartLineRAM * 8 + i * 8);
			tmp_string[6] = 0x20;
			Hexview((unsigned char *)(Ram_68k + M68k_StartLineRAM * 8 + i * 8), tmp_string + 7);
			tmp_string[24] = 0x20;
			Ansiview((unsigned char *)(Ram_68k + M68k_StartLineRAM * 8 + i * 8), tmp_string + 25);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hM68K, IDC_68K_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
		Byte_Swap(Ram_68k, Rom_Size);
	}
	wsprintf(debug_string, "X=%d N=%d Z=%d V=%d C=%d  SR=%.4X Cycles=%.10d", (main68k_context.sr & 0x10) ? 1 : 0, (main68k_context.sr & 0x8) ? 1 : 0, (main68k_context.sr & 0x4) ? 1 : 0, (main68k_context.sr & 0x2) ? 1 : 0, (main68k_context.sr & 0x1) ? 1 : 0, main68k_context.sr, main68k_context.odometer);
	SendDlgItemMessage(hM68K, IDC_68K_STATUS_SR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "A0=%.8X A1=%.8X A2=%.8X A3=%.8X\nA4=%.8X A5=%.8X A6=%.8X A7=%.8X\n", main68k_context.areg[0], main68k_context.areg[1], main68k_context.areg[2], main68k_context.areg[3], main68k_context.areg[4], main68k_context.areg[5], main68k_context.areg[6], main68k_context.areg[7]);
	SendDlgItemMessage(hM68K, IDC_68K_STATUS_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "D0=%.8X D1=%.8X D2=%.8X D3=%.8X\nD4=%.8X D5=%.8X D6=%.8X D7=%.8X\n", main68k_context.dreg[0], main68k_context.dreg[1], main68k_context.dreg[2], main68k_context.dreg[3], main68k_context.dreg[4], main68k_context.dreg[5], main68k_context.dreg[6], main68k_context.dreg[7]);
	SendDlgItemMessage(hM68K, IDC_68K_STATUS_DATA, WM_SETTEXT, 0, (LPARAM)debug_string);

	wsprintf(debug_string, "PC=%.8X", main68k_context.pc);
	SendDlgItemMessage(hM68K, IDC_68K_PC, WM_SETTEXT, (WPARAM)0, (LPARAM)debug_string);

	/*
	sprintf(GString, "Bank for Z80 = %.8X\n", Bank_Z80);
	*/
}


void Dump68KRom_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "ROM dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump Genesis ROM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = Rom_Size;
	Byte_Swap(Rom_Data, Rom_Size);
	WriteFile(hFr, Rom_Data, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Rom_Data, Rom_Size);

	CloseHandle(hFr);

	Put_Info("Genesis ROM dumped", 1500);
}


void Dump68K_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_68K");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 68K memory";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 64 * 1024;
	Byte_Swap(Ram_68k, Rom_Size);
	WriteFile(hFr, Ram_68k, dwBytesToWrite, &dwBytesWritten, NULL);
	Byte_Swap(Ram_68k, Rom_Size);

	CloseHandle(hFr);

	Put_Info("68K RAM dumped", 1500);
}

BOOL CALLBACK M68KDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	
	SCROLLINFO si;

	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SendDlgItemMessage(hM68K, IDC_68K_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hM68K, IDC_68K_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hM68K, IDC_68K_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hM68K, IDC_68K_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);

		m68kdebug_reset();
		break;

	case WM_SHOWWINDOW:
		SwitchM68kViewMode_KMod();
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_68K_DUMP_ROM:
			Dump68KRom_KMod(hwnd);
			break;
		case IDC_68K_DUMP_RAM:
			Dump68K_KMod(hwnd);
			break;
		case IDC_68K_VIEW_ROM:
			M68_ViewMode = !M68_ViewMode;
			if (M68_ViewMode == 0)
			{
				SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
			}
			else
			{
				SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
			}
			SwitchM68kViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateM68k_KMod();
			break;
		case IDC_68K_VIEW_RAM:
			M68_ViewMode = 2; //RAM
			SendDlgItemMessage(hwnd, IDC_68K_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
			SwitchM68kViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateM68k_KMod();
			break;

		case IDC_68K_PC:
			if (M68_ViewMode == 0)
			{
				M68k_StartLineDisasm = main68k_context.pc;
				SwitchM68kViewMode_KMod();
				UpdateWindow(hwnd);
				UpdateM68k_KMod();
			}
			break;

		}
		break;

	case WM_VSCROLL:
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL; //SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
		GetScrollInfo((HWND)lParam, SB_CTL, &si);
		switch (LOWORD(wParam))
		{
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_LINEUP:
			si.nPos--;
			break;
		case SB_LINEDOWN:
			si.nPos++;
			break;
			//				case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			// don't change!! else you'll loose the 32bits value for a 16bits value
			//					 si.nPos = HIWORD(wParam);
			si.nPos = si.nTrackPos;
			break;
		}

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		SetScrollInfo((HWND)lParam, SB_CTL, &si, TRUE);
		UpdateWindow(hwnd);
		UpdateM68k_KMod();
		return 0;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_68K);
		break;

	case WM_DESTROY:
		m68kdebug_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;

}


void m68kdebug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hM68K = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG68K), hWndParent, M68KDlgProc);
}

void m68kdebug_show(BOOL visibility)
{
	ShowWindow(hM68K, visibility ? SW_SHOW : SW_HIDE);
}

void m68kdebug_update()
{
	if (OpenedWindow_KMod[DMODE_68K-1] == FALSE)	return;

	UpdateM68k_KMod();
}

void m68kdebug_reset()
{
	M68_ViewMode = 0; //disasm
	M68k_StartLineDisasm = M68k_StartLineRAM = M68k_StartLineROM = 0;
	SwitchM68kViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void m68kdebug_destroy()
{
	DeleteObject((HGDIOBJ)hFont);
	DestroyWindow(hM68K);
}

void m68kdebug_dump()
{
	Dump68K_KMod(hM68K);
}
void m68kdebug_jumpRAM(DWORD adr)
{
	M68_ViewMode = 2; //RAM
	M68k_StartLineRAM = adr / 8;
	SwitchM68kViewMode_KMod();
}