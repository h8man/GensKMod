#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../Mem_M68k.h"
#include "../Mem_S68k.h"

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "cd_reg.h"

static HWND hCD_Reg, hCDRegList;

void CDRegInit_KMod(HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i;
	char		buf[64];
	RECT		rSize;
	TCHAR       szString[3][20] = { "Address (hex)", "MainCPU (0xA12000)", "SubCPU (0xFF8000)" };

	hCDRegList = GetDlgItem(hwnd, IDC_CD_REG_LIST);
	ListView_DeleteAllItems(hCDRegList);

	GetWindowRect(hCDRegList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80; // ((rSize.right - rSize.left -5)/3);
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(hCDRegList, 0, &lvColumn);

	lvColumn.cx = 0x75;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(hCDRegList, 1, &lvColumn);

	lvColumn.cx = 0x75;
	lvColumn.pszText = szString[2];
	ListView_InsertColumn(hCDRegList, 2, &lvColumn);

	ListView_SetExtendedListViewStyle(hCDRegList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	for (i = 0; i <= 0x4A; i += 2)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0;
		wsprintf(buf, "%0.2X", i);
		lvItem.pszText = buf;
		lvItem.iItem = i;
		ListView_InsertItem(hCDRegList, &lvItem);

		lvItem.iSubItem = 1;
		if (i > 0x2E)
			wsprintf(buf, "");
		else
			wsprintf(buf, "0x%0.4X", M68K_RW(0xA12000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);

		lvItem.iSubItem = 2;
		wsprintf(buf, "0x%0.4X", S68K_RW(0xFF8000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);
	}
}

void UpdateCD_Reg_KMod()
{
	int         i;
	char		buf[64];
	LVITEM		lvItem;

	for (i = 0; i <= 0x4A; i += 2)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i / 2;

		lvItem.iSubItem = 1;
		if (i > 0x2E)
			wsprintf(buf, "");
		else
			wsprintf(buf, "0x%0.4X", M68K_RW(0xA12000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);

		lvItem.iSubItem = 2;
		wsprintf(buf, "0x%0.4X", S68K_RW(0xFF8000 | i));
		lvItem.pszText = buf;
		ListView_SetItem(hCDRegList, &lvItem);
	}
}


BOOL CALLBACK CD_RegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_INITDIALOG:
		CDRegInit_KMod(hwnd);
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_CD_REG);
		break;

	case WM_DESTROY:
		DestroyWindow(hCD_Reg);
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


void cdreg_create(HINSTANCE hInstance, HWND hWndParent)
{
	hCD_Reg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGCD_REG), hWndParent, CD_RegDlgProc);
}

void cdreg_show(BOOL visibility)
{
	ShowWindow(hCD_Reg, visibility ? SW_SHOW : SW_HIDE);
}

void cdreg_update()
{
	if (OpenedWindow_KMod[DMODE_CD_REG-1] == FALSE)	return;

	UpdateCD_Reg_KMod();
}

void cdreg_reset()
{
	CDRegInit_KMod(hCD_Reg);
}
void cdreg_destroy()
{
	DestroyWindow(hCD_Reg);
}

