#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../LC89510.h"

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "cdc.h"

static HWND hCD_CDC;
HWND hCDCDCList;

void CD_CDCInit_KMod(HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	//	int         i;
	char		buf[64];
	RECT		rSize;
	TCHAR       szString[2][20] = { "Name", "Value" };

	hCDCDCList = GetDlgItem(hwnd, IDC_CDC_LIST);
	ListView_DeleteAllItems(hCDCDCList);

	GetWindowRect(hCDCDCList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80;
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(hCDCDCList, 0, &lvColumn);

	lvColumn.cx = 80;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(hCDCDCList, 1, &lvColumn);


	ListView_SetExtendedListViewStyle(hCDCDCList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	lvItem.mask = LVIF_TEXT;

	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = "COMIN";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.COMIN);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 1;
	lvItem.iSubItem = 0;
	lvItem.pszText = "IFSTAT";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.IFSTAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 2;
	lvItem.iSubItem = 0;
	lvItem.pszText = "DBC";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.DBC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 3;
	lvItem.iSubItem = 0;
	lvItem.pszText = "HEAD";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.HEAD);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 4;
	lvItem.iSubItem = 0;
	lvItem.pszText = "PT";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.PT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 5;
	lvItem.iSubItem = 0;
	lvItem.pszText = "WA";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.WA);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 6;
	lvItem.iSubItem = 0;
	lvItem.pszText = "STAT";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.STAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 7;
	lvItem.iSubItem = 0;
	lvItem.pszText = "CTRL";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.CTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 8;
	lvItem.iSubItem = 0;
	lvItem.pszText = "DAC";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.DAC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 9;
	lvItem.iSubItem = 0;
	lvItem.pszText = "IFCTRL";
	ListView_InsertItem(hCDCDCList, &lvItem);
	//	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.IFCTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
}

void UpdateCD_CDC_KMod()
{
	//	int         i;
	char		buf[64];
	LVITEM		lvItem;
	
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;


	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 0;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.COMIN);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 1;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.IFSTAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 2;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.DBC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 3;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.HEAD);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 4;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.PT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 5;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.WA);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 6;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.STAT);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 7;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.8X", CDC.CTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 8;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.4X", CDC.DAC);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);

	//	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = 9;
	//	lvItem.iSubItem = 1;
	wsprintf(buf, "0x%0.2X", CDC.IFCTRL);
	lvItem.pszText = buf;
	ListView_SetItem(hCDCDCList, &lvItem);
}


BOOL CALLBACK CD_CDCDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_INITDIALOG:
		CD_CDCInit_KMod(hwnd);
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_CD_CDC);
		break;

	case WM_DESTROY:
		cdcdebug_destroy();

		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void cdcdebug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hCD_CDC = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGCD_CDC), hWndParent, CD_CDCDlgProc);
}

void cdcdebug_show(BOOL visibility)
{
	ShowWindow(hCD_CDC, visibility ? SW_SHOW : SW_HIDE);
}

void cdcdebug_update()
{
	if (OpenedWindow_KMod[DMODE_CD_CDC-1] == FALSE)	return;

	UpdateCD_CDC_KMod();
}

void cdcdebug_reset()
{
	//TODO remove and create a real reset (all to 0) ?
	CD_CDCInit_KMod(hCD_CDC);
}
void cdcdebug_destroy()
{
	DestroyWindow(hCD_CDC);
}

