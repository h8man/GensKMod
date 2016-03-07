#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h"
#include "../resource.h"
#include "../ym2612.h"
#include "../z80.h" //for M_Z80 & z80_Read_Odo
#include "../mem_M68K.h"

#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "config.h"
#include "ym2612.h"



struct oper {
	unsigned char op_data[8];
	long op_padding[2];
};

struct voice {
	struct oper op[4];
	unsigned char	algo;
	unsigned char	fback;
	unsigned char 	padding[14];
};

struct ym2612_KMod {
	struct 		voice ym;
	unsigned char	name[16];
	unsigned char	dumper[16];
	unsigned char	game[16];
}KM2612;




static HWND hYM2612;

HWND hTabYM2612;
UINT  notes[6][4];
UCHAR curAlgo;
/*RECT rcAlgo;*/
DWORD KDAC_ticks;
int KDAC_freq;
CHAR debug_string[1024];

//TODO rename
void SpyYM2612DAC()
{
	DWORD curTicks;
	curTicks = z80_Read_Odo(&M_Z80);
	if (curTicks != KDAC_ticks)
	{
		if (CPU_Mode)
			KDAC_freq = Round_Double(((double)CLOCK_PAL / 15.0) / (curTicks - KDAC_ticks));
		else
			KDAC_freq = Round_Double(((double)CLOCK_NTSC / 15.0) / (curTicks - KDAC_ticks));
	}
	KDAC_ticks = curTicks;

}

//called every frame
void UpdateYM2612_KMod()
{
	int timer;
	UCHAR curSel, op, chan, part, algo;
	UCHAR i, j;
	CHAR tmp_string[30];
	HBITMAP hBitmap;

	switch (YM2612.REG[0][0x28] & 0x07)
	{
	case 0:
		chan = 0;
		break;
	case 1:
		chan = 1;
		break;
	case 2:
		chan = 2;
		break;
	case 4:
		chan = 3;
		break;
	case 5:
		chan = 4;
		break;
	case 6:
		chan = 5;
		break;
	default:
		chan = 0;

	}

	op = YM2612.REG[0][0x28] & 0xF0;
	op >>= 4;

	if (op & 0x01)
		notes[chan][0] = BST_CHECKED;
	else
		notes[chan][0] = BST_UNCHECKED;

	if (op & 0x02)
		notes[chan][1] = BST_CHECKED;
	else
		notes[chan][1] = BST_UNCHECKED;

	if (op & 0x04)
		notes[chan][2] = BST_CHECKED;
	else
		notes[chan][2] = BST_UNCHECKED;

	if (op & 0x08)
		notes[chan][3] = BST_CHECKED;
	else
		notes[chan][3] = BST_UNCHECKED;


	if (OpenedWindow_KMod[11] == FALSE)	return;


	curSel = TabCtrl_GetCurSel(hTabYM2612);


	part = 0;
	chan = curSel % 3;
	if (curSel >= 3)	part = 1;



	/* Operators */
	for (op = 0; op<4; op++)
	{
		wsprintf(debug_string, "0x%0.2X\n", YM2612.REG[part][0x30 + chan + 4 * op] >> 4);
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x30 + chan + 4 * op] & 0x0F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x40 + chan + 4 * op] & 0x7F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x50 + chan + 4 * op] >> 6);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x50 + chan + 4 * op] & 0x1F);
		lstrcat(debug_string, tmp_string);
		if (YM2612.REG[0][0x60] & 0x80)
			lstrcat(debug_string, "ON\n");
		else
			lstrcat(debug_string, "OFF\n");
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x60 + chan + 4 * op] & 0x1F);
		lstrcat(debug_string, tmp_string);
		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x70 + chan + 4 * op] & 0x1F);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x80 + chan + 4 * op] & 0xF0 >> 4);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0x80 + chan + 4 * op] & 0x0F);
		lstrcat(debug_string, tmp_string);

		wsprintf(tmp_string, "0x%0.2X", YM2612.REG[part][0x90 + chan + 4 * op] & 0xFF);
		lstrcat(debug_string, tmp_string);

		SetDlgItemText(hYM2612, IDC_YM2612_OP1 + op, debug_string);
	}


	/* Channel */
	wsprintf(debug_string, "0x%0.2X%0.2X\n", YM2612.REG[part][0xA4 + chan] & 0x07, YM2612.REG[part][0xA0 + chan]);
	wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0xA4 + chan] & 0x38 >> 2);
	lstrcat(debug_string, tmp_string);
	SetDlgItemText(hYM2612, IDC_YM2612_FREQ, debug_string);

	wsprintf(debug_string, "0x%0.2X\n", (YM2612.REG[part][0xB4 + chan] & 0x30) >> 4); //AMS
	wsprintf(tmp_string, "0x%0.2X\n", YM2612.REG[part][0xB4 + chan] & 0x07); //FMS
	lstrcat(debug_string, tmp_string);
	SetDlgItemText(hYM2612, IDC_YM2612_MOD, debug_string);

	if (YM2612.REG[part][0xB4 + chan] & 0x80)
		CheckDlgButton(hYM2612, IDC_YM2612_LEFT, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_LEFT, BST_UNCHECKED);

	if (YM2612.REG[part][0xB4 + chan] & 0x40)
		CheckDlgButton(hYM2612, IDC_YM2612_RIGHT, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_RIGHT, BST_UNCHECKED);

	wsprintf(debug_string, "0x%0.2X", (YM2612.REG[part][0xB0 + chan] & 0x38) >> 3);
	SetDlgItemText(hYM2612, IDC_YM2612_FBACK, debug_string);

	algo = YM2612.REG[part][0xB0 + chan] & 0x07;
	/* optimization to avoid flickers */
	if (algo != curAlgo)
	{
		hBitmap = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_ALGO0 + algo));
		SendDlgItemMessage(hYM2612, IDC_YM2612_ALGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		curAlgo = algo;
	}


	/* global */
	if (YM2612.REG[0][0x22] & 0x08)
	{
		CheckDlgButton(hYM2612, IDC_YM2612_LFO, BST_CHECKED);
		wsprintf(debug_string, "LFO 0x%0.2X", YM2612.REG[0][0x22] & 0x07);
	}
	else
	{
		CheckDlgButton(hYM2612, IDC_YM2612_LFO, BST_UNCHECKED);
		lstrcpy(debug_string, "LFO");
	}

	SetDlgItemText(hYM2612, IDC_YM2612_LFO, debug_string);

	timer = YM2612.REG[0][0x24];
	timer <<= 2;
	timer |= YM2612.REG[0][0x25];
	wsprintf(debug_string, "0x%0.4X\n0x%0.2X", timer, YM2612.REG[0][0x26]);
	SetDlgItemText(hYM2612, IDC_YM2612_TIMER, debug_string);

	if (YM2612.REG[0][0x27] & 0x01)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A1, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A1, BST_UNCHECKED);

	if (YM2612.REG[0][0x27] & 0x02)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B1, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B1, BST_UNCHECKED);

	if (YM2612.REG[0][0x27] & 0x04)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A2, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A2, BST_UNCHECKED);

	if (YM2612.REG[0][0x27] & 0x08)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B2, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B2, BST_UNCHECKED);

	if (YM2612.REG[0][0x27] & 0x10)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A3, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_A3, BST_UNCHECKED);

	if (YM2612.REG[0][0x27] & 0x20)
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B3, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_TIMER_B3, BST_UNCHECKED);


	switch (YM2612.REG[0][0x27] & 0xC0)
	{
	case 0:
		lstrcpy(debug_string, "Normal");
		break;
	case 0x40:
		lstrcpy(debug_string, "Special");
		break;
	case 0x80:
	case 0xC0:
	default:
		lstrcpy(debug_string, "Illegal");

	}
	SetDlgItemText(hYM2612, IDC_YM2612_C3MODE, debug_string);


	for (i = 0; i<6; i++)
	{
		for (j = 0; j<4; j++)
		{
			CheckDlgButton(hYM2612, IDC_YM2612_KEY_1_1 + j + i * 4, notes[i][j]);
		}
	}


	if (YM2612.DAC)
	{
		CheckDlgButton(hYM2612, IDC_YM2612_DAC, BST_CHECKED);
		wsprintf(debug_string, "DAC 0x%0.2X at %0.5dHz", ((YM2612.DACdata) >> 7) & 0xFF, KDAC_freq);
	}
	else
	{
		CheckDlgButton(hYM2612, IDC_YM2612_DAC, BST_UNCHECKED);
		lstrcpy(debug_string, "DAC");
	}

	SetDlgItemText(hYM2612, IDC_YM2612_DAC, debug_string);
}


void YM2612_ChangeChannel(HWND hwnd)
{
	int curSel = TabCtrl_GetCurSel(hTabYM2612);

	/* enabled */
	if (EnabledChannels[curSel] == TRUE)
		CheckDlgButton(hYM2612, IDC_YM2612_MUTE, BST_CHECKED);
	else
		CheckDlgButton(hYM2612, IDC_YM2612_MUTE, BST_UNCHECKED);

	UpdateYM2612_KMod();
}

void YM2612_ToggleMute_KMod(HWND hwnd)
{
	int tabIndex;
	TCITEM tab;

	tabIndex = TabCtrl_GetCurSel(hTabYM2612);

	tab.mask = TCIF_IMAGE;
	if (IsDlgButtonChecked(hYM2612, IDC_YM2612_MUTE) == BST_CHECKED)
	{
		EnabledChannels[tabIndex] = TRUE;
		tab.iImage = 0;
	}
	else
	{
		EnabledChannels[tabIndex] = FALSE;
		tab.iImage = 1;
	}

	TabCtrl_SetItem(hTabYM2612, tabIndex, &tab);
}

void DumpYM2612_KMod(HWND hwnd)
{
	OPENFILENAME		szFile;
	CHAR				szFileName[MAX_PATH];
	HANDLE				hFr;
	DWORD				dwBytesToWrite, dwBytesWritten;
	UCHAR				curSel, op, chan, part, tmp;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_Chan");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "YM2612 voice dump (*.y12)\0*.y12\0TFM'ucker instrument (*.tfi)\0*.tfi\0VED voice dump (*.ff)\0*.ff\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump YM2612 voice";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "y12";

	if (GetSaveFileName(&szFile) != TRUE)   return;



	curSel = TabCtrl_GetCurSel(hTabYM2612);
	part = 0;
	chan = curSel % 3;
	if (curSel >= 3)	part = 1;

	hFr = CreateFile(szFileName, GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hFr == INVALID_HANDLE_VALUE)
	{
		return;
	}


	ZeroMemory(&KM2612, sizeof(szFile));

	/* operators data */
	for (op = 0; op<4; op++)
	{
		KM2612.ym.op[op].op_data[0] = (UCHAR)(YM2612.REG[part][0x30 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[1] = (UCHAR)(YM2612.REG[part][0x40 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[2] = (UCHAR)(YM2612.REG[part][0x50 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[3] = (UCHAR)(YM2612.REG[part][0x60 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[4] = (UCHAR)(YM2612.REG[part][0x70 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[5] = (UCHAR)(YM2612.REG[part][0x80 + chan + 4 * op]) & 0xFF;
		KM2612.ym.op[op].op_data[6] = (UCHAR)(YM2612.REG[part][0x90 + chan + 4 * op]) & 0xFF;
	}

	/* channel data */
	KM2612.ym.algo = (UCHAR)(YM2612.REG[part][0xB0 + chan]) & 0x07;
	KM2612.ym.fback = (UCHAR)(YM2612.REG[part][0xB0 + chan]) & 0x38;
	KM2612.ym.fback >>= 3;

	if (szFile.nFilterIndex == 1)
	{
		/* y12 */
		strncpy(KM2612.name, Rom_Name, 16);
		strncpy(KM2612.dumper, Rom_Name, 16);
		strncpy(KM2612.game, Rom_Name, 16);

		dwBytesToWrite = sizeof(KM2612);
		WriteFile(hFr, &KM2612, dwBytesToWrite, &dwBytesWritten, NULL);
	}
	else if (szFile.nFilterIndex == 2)
	{
		/* tfi */
		dwBytesToWrite = 1;
		WriteFile(hFr, &KM2612.ym.algo, dwBytesToWrite, &dwBytesWritten, NULL);
		WriteFile(hFr, &KM2612.ym.fback, dwBytesToWrite, &dwBytesWritten, NULL);


		for (op = 0; op<4; op++)
		{
			dwBytesToWrite = 1;
			//MUL
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[0]) & 0xF;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);
			//DT1
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[0]) & 0x70;
			tmp >>= 4;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//TL
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[1]) & 0x7F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//RS
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[2]) & 0xC0;
			tmp >>= 6;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);
			//AR
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[2]) & 0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//D1R
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[3]) & 0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//D2R
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[4]) & 0x1F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//RR
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[5]) & 0x0F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);
			//D1L
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[5]) & 0xF0;
			tmp >>= 4;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

			//SSG-EG
			tmp = (UCHAR)(KM2612.ym.op[op].op_data[6]) & 0x0F;
			WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

		}
	}
	else if (szFile.nFilterIndex == 3)
	{
		/* ff */
		for (tmp = 0; tmp<6; tmp++)
		{
			for (op = 0; op<4; op++)
			{
				dwBytesToWrite = 1;
				WriteFile(hFr, &KM2612.ym.op[op].op_data[tmp], dwBytesToWrite, &dwBytesWritten, NULL);
			}
		}

		tmp = (UCHAR)(YM2612.REG[part][0xB0 + chan]) & 0xFF;
		dwBytesToWrite = 1;
		WriteFile(hFr, &tmp, dwBytesToWrite, &dwBytesWritten, NULL);

		strncpy(KM2612.name, Rom_Name, 7);
		dwBytesToWrite = 7;
		WriteFile(hFr, KM2612.name, dwBytesToWrite, &dwBytesWritten, NULL);
	}

	CloseHandle(hFr);

	Put_Info("YM2616 voice dumped", 1500);
}


BOOL CALLBACK YM2612DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		HIMAGELIST himl = NULL;
		TCITEM tie;
		int i;

		curAlgo = 0;
		KDAC_ticks = 0;
		KDAC_freq = 0;

		ZeroMemory(&tie, sizeof(TCITEM));
		hTabYM2612 = GetDlgItem(hwnd, IDC_YM2612_TAB);

		TabCtrl_DeleteAllItems(hTabYM2612);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 1";
		i = TabCtrl_InsertItem(hTabYM2612, 0, &tie);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 2";
		i = TabCtrl_InsertItem(hTabYM2612, 1, &tie);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 3";
		i = TabCtrl_InsertItem(hTabYM2612, 2, &tie);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 4";
		i = TabCtrl_InsertItem(hTabYM2612, 3, &tie);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 5";
		i = TabCtrl_InsertItem(hTabYM2612, 4, &tie);

		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = 0;
		tie.pszText = "Channel 6";
		i = TabCtrl_InsertItem(hTabYM2612, 5, &tie);


		/*			GetWindowRect( GetDlgItem(hwnd, IDC_YM2612_ALGO), &rcAlgo );*/
		himl = ImageList_LoadImage(ghInstance, MAKEINTRESOURCE(IDB_YM2612_IMAGELIST), 16, 0, IMAGE_BITMAP, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);
		if (himl == NULL)
			return -1;

		TabCtrl_SetImageList(hTabYM2612, himl);

		//enabled by default
		CheckDlgButton(hwnd, IDC_YM2612_MUTE, BST_CHECKED);
	}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_YM2612_DUMP)
			DumpYM2612_KMod(hwnd);
		else if (LOWORD(wParam) == IDC_YM2612_MUTE)
			YM2612_ToggleMute_KMod(hwnd);
		break;

	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->code)
		{
		case TCN_SELCHANGE:
			YM2612_ChangeChannel(hwnd);
			break;

		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_YM2612);
		break;

	case WM_DESTROY:
		ym2612_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void ym2612_create(HINSTANCE hInstance, HWND hWndParent)
{
	hYM2612 = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGYM2612), hWndParent, YM2612DlgProc);
}

void ym2612_show(BOOL visibility)
{
	ShowWindow(hYM2612, visibility ? SW_SHOW : SW_HIDE);
}

void ym2612_update()
{
	if (OpenedWindow_KMod[DMODE_YM2612 - 1] == FALSE)	return;

	UpdateYM2612_KMod();
}

void ym2612_reset()
{
	char i,j;
	for (i = 0; i<6; i++)
	{
		EnabledChannels[i] = TRUE;
		for (j = 0; j<4; j++)
		{
			notes[i][j] = BST_UNCHECKED;
		}
	}
}
void ym2612_destroy()
{
	DestroyWindow(hYM2612);
}
