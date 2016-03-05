#include <windows.h>
#include <commctrl.h>
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

static HWND hMSH2;
static unsigned char MSH2_ViewMode;
static unsigned int  MSH2_StartLineROMDisasm, MSH2_StartLineRAMDisasm, MSH2_StartLineRAM, MSH2_StartLineROM, MSH2_StartLineCache;
static CHAR debug_string[1024];

void UpdateMSH2_KMod()
{
	unsigned int i, PC;
	unsigned char tmp_string[256];
	
	SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	if (MSH2_ViewMode & 2)
	{
		if (MSH2_ViewMode & 1)
		{
			// ROM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			MSH2_StartLineROMDisasm = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL);
			PC = 0x02000000 + MSH2_StartLineROMDisasm * 2;
			for (i = 0; i < 13; PC += 2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
				SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)tmp_string);
			}
		}
		else
		{
			// ROM view
			MSH2_StartLineROM = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL);
			for (i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x02000000 + MSH2_StartLineROM * 8 + i * 8);
				tmp_string[8] = 0x20;
				Hexview((unsigned char *)(_32X_Rom + MSH2_StartLineROM * 8 + i * 8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview((unsigned char *)(_32X_Rom + MSH2_StartLineROM * 8 + i * 8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
			}
		}
	}
	else if (MSH2_ViewMode & 8)
	{
		if (MSH2_ViewMode & 4)
		{
			// RAM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			MSH2_StartLineRAMDisasm = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL);
			PC = 0x06000000 + MSH2_StartLineRAMDisasm * 2;
			for (i = 0; i < 13; PC += 2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
				SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)tmp_string);
			}
		}
		else
		{
			// RAM view
			MSH2_StartLineRAM = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL);
			for (i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x06000000 + MSH2_StartLineRAM * 8 + i * 8);
				tmp_string[8] = 0x20;
				Hexview((unsigned char *)(_32X_Ram + MSH2_StartLineRAM * 8 + i * 8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview((unsigned char *)(_32X_Ram + MSH2_StartLineRAM * 8 + i * 8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
			}
		}
	}
	else if (MSH2_ViewMode & 0x20)
	{
		/*
		if (MSH2_ViewMode&0x10)
		{
		// Cache disasm
		MSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL) ,SB_CTL);
		PC = 0x06000000 + MSH2_StartLineRAMDisasm*2;
		for(i = 0; i < 13; PC+=2, i++)
		{
		SH2Disasm(tmp_string, PC, SH2_Read_Word(&M_SH2, PC), 0);
		SendDlgItemMessage(hMSH2 , IDC_MSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
		}
		}
		else
		{*/
		// Cache view
		MSH2_StartLineCache = GetScrollPos(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL);
		for (i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.8X ", 0xC0000000 + MSH2_StartLineCache * 8 + i * 8);
			tmp_string[8] = 0x20;
			Hexview((unsigned char *)(M_SH2.Cache + MSH2_StartLineCache * 8 + i * 8), tmp_string + 9);
			tmp_string[26] = 0x20;
			Ansiview((unsigned char *)(M_SH2.Cache + MSH2_StartLineCache * 8 + i * 8), tmp_string + 27);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hMSH2, IDC_MSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
		//	}
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
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nPage = 10;

	if (MSH2_ViewMode & 2)
	{
		if (MSH2_ViewMode & 1)
		{
			// ROM DISASM VIEW
			si.nPos = MSH2_StartLineROMDisasm;
			si.nMax = ((4 * 1024 * 1024) / 2) - 1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
		}
		else
		{
			// ROM VIEW
			si.nPos = MSH2_StartLineROM;
			si.nMax = ((4 * 1024 * 1024) / 8) - 1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (MSH2_ViewMode & 8)
	{
		if (MSH2_ViewMode & 4)
		{
			// RAM DISASM VIEW
			si.nPos = MSH2_StartLineRAMDisasm;
			si.nMax = ((256 * 1024) / 2) - 1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{
			// RAM VIEW
			si.nPos = MSH2_StartLineRAM;
			si.nMax = ((256 * 1024) / 8) - 1;
			SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (MSH2_ViewMode & 0x20)
	{
		/*
		if (MSH2_ViewMode&0x10)
		{
		// CACHE DISASM VIEW
		si.nPos   = MSH2_StartLineRAMDisasm;
		si.nMax   = ((256 * 1024)/2) -1;
		SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{*/
		// CACHE VIEW
		si.nPos = MSH2_StartLineCache;
		si.nMax = (0x1000 / 8) - 1;
		/*
		SendDlgItemMessage(hMSH2, IDC_MSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}*/
	}
	SetScrollInfo(GetDlgItem(hMSH2, IDC_MSH2_SCROLL), SB_CTL, &si, TRUE);
}

BOOL CALLBACK MSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si;
	unsigned int curPC;
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_MSH2_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_MSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);

		mSH2_reset();
		break;

	case WM_SHOWWINDOW:
		SwitchMSH2ViewMode_KMod();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
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
			UpdateWindow(hwnd);
			UpdateMSH2_KMod();
			break;
		case IDC_MSH2_VIEW_RAM:
			MSH2_ViewMode &= 0x15;
			MSH2_ViewMode ^= 0x4;
			MSH2_ViewMode |= 0x8;

			SwitchMSH2ViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateMSH2_KMod();
			break;
		case IDC_MSH2_VIEW_CACHE:
			MSH2_ViewMode &= 0x15;
			//					MSH2_ViewMode ^= 0x10;
			MSH2_ViewMode |= 0x20;

			SwitchMSH2ViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateMSH2_KMod();
			break;

		case IDC_MSH2_PC:
			curPC = SH2_Get_PC(&M_SH2); //(M_SH2.PC - M_SH2.Base_PC) - 4;
			if (curPC < 0x2400000)
			{
				MSH2_ViewMode &= 0x15;
				MSH2_ViewMode |= 0x2;
				MSH2_StartLineROM = (curPC - 0x02000000) / 8;
				MSH2_StartLineROMDisasm = (curPC - 0x02000000) / 2;
				SwitchMSH2ViewMode_KMod();
			}
			else if (curPC < 0x6040000)
			{
				MSH2_ViewMode &= 0x15;
				MSH2_ViewMode |= 0x8;
				MSH2_StartLineRAM = (curPC - 0x06000000) / 8;
				MSH2_StartLineRAMDisasm = (curPC - 0x06000000) / 2;
				SwitchMSH2ViewMode_KMod();
			}

			UpdateWindow(hwnd);
			UpdateMSH2_KMod();
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
		UpdateMSH2_KMod();
		return 0;

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
	MSH2_StartLineROMDisasm = MSH2_StartLineROM = MSH2_StartLineRAMDisasm = MSH2_StartLineRAM = MSH2_StartLineCache = 0;

	SwitchMSH2ViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void mSH2_destroy()
{
	DestroyWindow(hMSH2);
}

