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
#include "sSH2.h"

static HWND hSSH2;
static unsigned char SSH2_ViewMode;
static unsigned int  SSH2_StartLineROMDisasm, SSH2_StartLineRAMDisasm, SSH2_StartLineRAM, SSH2_StartLineROM, SSH2_StartLineCache;
static CHAR debug_string[1024];

void UpdateSSH2_KMod()
{
	unsigned int i, PC;
	unsigned char tmp_string[256];

	SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	if (SSH2_ViewMode & 2)
	{
		if (SSH2_ViewMode & 1)
		{
			// ROM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			SSH2_StartLineROMDisasm = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL);
			PC = 0x02000000 + SSH2_StartLineROMDisasm * 2;
			for (i = 0; i < 13; PC += 2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
				SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)tmp_string);
			}
		}
		else
		{
			// ROM view
			SSH2_StartLineROM = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL);
			for (i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x02000000 + SSH2_StartLineROM * 8 + i * 8);
				tmp_string[8] = 0x20;
				Hexview((unsigned char *)(_32X_Rom + SSH2_StartLineROM * 8 + i * 8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview((unsigned char *)(_32X_Rom + SSH2_StartLineROM * 8 + i * 8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
			}
		}
	}
	else if (SSH2_ViewMode & 8)
	{
		if (SSH2_ViewMode & 4)
		{
			// RAM disasm
			//PC = (sh->PC - sh->Base_PC) - 4;
			SSH2_StartLineRAMDisasm = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL);
			PC = 0x06000000 + SSH2_StartLineRAMDisasm * 2;
			for (i = 0; i < 13; PC += 2, i++)
			{
				SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
				SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)tmp_string);
			}
		}
		else
		{
			// RAM view
			SSH2_StartLineRAM = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL);
			for (i = 0; i < 13; i++)
			{
				wsprintf(tmp_string, "%.8X ", 0x06000000 + SSH2_StartLineRAM * 8 + i * 8);
				tmp_string[8] = 0x20;
				Hexview((unsigned char *)(_32X_Ram + SSH2_StartLineRAM * 8 + i * 8), tmp_string + 9);
				tmp_string[26] = 0x20;
				Ansiview((unsigned char *)(_32X_Ram + SSH2_StartLineRAM * 8 + i * 8), tmp_string + 27);
				wsprintf(debug_string, "%s", tmp_string);
				SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
			}
		}
	}
	else if (SSH2_ViewMode & 0x20)
	{
		/*
		if (SSH2_ViewMode&0x10)
		{
		// Cache disasm
		SSH2_StartLineRAMDisasm =	GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL) ,SB_CTL);
		PC = 0x06000000 + SSH2_StartLineRAMDisasm*2;
		for(i = 0; i < 13; PC+=2, i++)
		{
		SH2Disasm(tmp_string, PC, SH2_Read_Word(&S_SH2, PC), 0);
		SendDlgItemMessage(hSSH2 , IDC_SSH2_DISAM, LB_INSERTSTRING, i , (LPARAM)tmp_string);
		}
		}
		else
		{*/
		// Cache view
		SSH2_StartLineCache = GetScrollPos(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL);
		for (i = 0; i < 13; i++)
		{
			wsprintf(tmp_string, "%.8X ", 0xC0000000 + SSH2_StartLineCache * 8 + i * 8);
			tmp_string[8] = 0x20;
			Hexview((unsigned char *)(S_SH2.Cache + SSH2_StartLineCache * 8 + i * 8), tmp_string + 9);
			tmp_string[26] = 0x20;
			Ansiview((unsigned char *)(S_SH2.Cache + SSH2_StartLineCache * 8 + i * 8), tmp_string + 27);
			wsprintf(debug_string, "%s", tmp_string);
			SendDlgItemMessage(hSSH2, IDC_SSH2_DISAM, LB_INSERTSTRING, i, (LPARAM)debug_string);
		}
		//	}
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
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nPage = 10;

	if (SSH2_ViewMode & 2)
	{
		if (SSH2_ViewMode & 1)
		{
			// ROM DISASM VIEW
			si.nPos = SSH2_StartLineROMDisasm;
			si.nMax = ((1 * 1024) / 2) - 1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View ROM");
		}
		else
		{
			// ROM VIEW
			si.nPos = SSH2_StartLineROM;
			si.nMax = ((1 * 1024) / 8) - 1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_ROM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (SSH2_ViewMode & 8)
	{
		if (SSH2_ViewMode & 4)
		{
			// RAM DISASM VIEW
			si.nPos = SSH2_StartLineRAMDisasm;
			si.nMax = ((256 * 1024) / 2) - 1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{
			// RAM VIEW
			si.nPos = SSH2_StartLineRAM;
			si.nMax = ((256 * 1024) / 8) - 1;
			SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}
	}
	else if (SSH2_ViewMode & 0x20)
	{
		/*
		if (SSH2_ViewMode&0x10)
		{
		// CACHE DISASM VIEW
		si.nPos   = SSH2_StartLineRAMDisasm;
		si.nMax   = ((256 * 1024)/2) -1;
		SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View RAM");
		}
		else
		{*/
		// CACHE VIEW
		si.nPos = SSH2_StartLineCache;
		si.nMax = (0x1000 / 8) - 1;
		/*
		SendDlgItemMessage(hSSH2, IDC_SSH2_VIEW_RAM, WM_SETTEXT, (WPARAM)0, (LPARAM)"View Disasm");
		}*/
	}
	SetScrollInfo(GetDlgItem(hSSH2, IDC_SSH2_SCROLL), SB_CTL, &si, TRUE);
}

BOOL CALLBACK SSH2DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HFONT hFont = NULL;
	SCROLLINFO si;
	unsigned int curPC;
	switch (Message)
	{
	case WM_INITDIALOG:
		hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
		SendDlgItemMessage(hwnd, IDC_SSH2_DISAM, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_SR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_ADR, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendDlgItemMessage(hwnd, IDC_SSH2_STATUS_DATA, WM_SETFONT, (WPARAM)hFont, TRUE);
		sSH2_reset();
		
		break;

	case WM_SHOWWINDOW:
		SwitchSSH2ViewMode_KMod();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
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
			UpdateWindow(hwnd);
			UpdateSSH2_KMod();
			break;
		case IDC_SSH2_VIEW_RAM:
			SSH2_ViewMode &= 0x15;
			SSH2_ViewMode ^= 0x4;
			SSH2_ViewMode |= 0x8;

			SwitchSSH2ViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateSSH2_KMod();
			break;
		case IDC_SSH2_VIEW_CACHE:
			SSH2_ViewMode &= 0x15;
			//					SSH2_ViewMode ^= 0x10;
			SSH2_ViewMode |= 0x20;

			SwitchSSH2ViewMode_KMod();
			UpdateWindow(hwnd);
			UpdateSSH2_KMod();
			break;

		case IDC_SSH2_PC:
			curPC = SH2_Get_PC(&S_SH2); //(S_SH2.PC - S_SH2.Base_PC) - 4;
			if (curPC < 0x2400000)
			{
				SSH2_ViewMode &= 0x15;
				SSH2_ViewMode |= 0x2;
				SSH2_StartLineROM = (curPC - 0x02000000) / 8;
				SSH2_StartLineROMDisasm = (curPC - 0x02000000) / 2;
				SwitchSSH2ViewMode_KMod();
			}
			else if (curPC < 0x6040000)
			{
				SSH2_ViewMode &= 0x15;
				SSH2_ViewMode |= 0x8;
				SSH2_StartLineRAM = (curPC - 0x6000000) / 8;
				SSH2_StartLineRAMDisasm = (curPC - 0x6000000) / 2;
				SwitchSSH2ViewMode_KMod();
			}

			UpdateWindow(hwnd);
			UpdateSSH2_KMod();
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
		UpdateSSH2_KMod();
		return 0;

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
	SSH2_StartLineROMDisasm = SSH2_StartLineROM = SSH2_StartLineRAMDisasm = SSH2_StartLineRAM = SSH2_StartLineCache = 0;
	SwitchSSH2ViewMode_KMod(); // init with wrong values (since no game loaded by default)
}
void sSH2_destroy()
{
	DestroyWindow(hSSH2);
}

