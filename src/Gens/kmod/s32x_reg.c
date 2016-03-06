#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../Mem_SH2.h"
#include "../mem_M68K.h"

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "s32x_reg.h"

static HWND h32X_Reg;
static HWND h32XRegList;



struct _32X_register_struct
{
	BYTE side; //bit 0:MD, bit 1:32X
	WORD adr;
	char *descriptionMD;
	char *description32X;
};



const struct _32X_register_struct _32X_register[] =
{
	{ 3, 0x00, "Adapter control register", "Interrupt Mask" },
	{ 3, 0x02, "Interrupt control register", "Stand By change" },
	{ 3, 0x04, "Bank set register", "H Count" },
	{ 3, 0x06, "DREQ control register", "DREQ control register" },
	{ 3, 0x08, "DREQ source MSB", "DREQ source MSB" },
	{ 3, 0x0A, "DREQ source LSB", "DREQ source LSB" },
	{ 3, 0x0C, "DREQ destination MSB", "DREQ destination MSB" },
	{ 3, 0x0E, "DREQ destination LSB", "DREQ destination LSB" },
	{ 3, 0x10, "DREQ length", "DREQ length" },
	{ 3, 0x12, "FIFO", "FIFO" },
	{ 2, 0x14, "", "VRES Interrupt clear" },
	{ 2, 0x16, "", "V Interrupt clear" },
	{ 2, 0x18, "", "H Interrupt clear" },
	{ 3, 0x1A, "SEGA TV register", "CMD Interrupt clear" },
	{ 2, 0x1C, "", "PWM Interrupt clear" },
	{ 3, 0x20, "Comm Port 0", "Comm Port 0" },
	{ 3, 0x22, "Comm Port 1", "Comm Port 1" },
	{ 3, 0x24, "Comm Port 2", "Comm Port 2" },
	{ 3, 0x26, "Comm Port 3", "Comm Port 3" },
	{ 3, 0x28, "Comm Port 4", "Comm Port 4" },
	{ 3, 0x2A, "Comm Port 5", "Comm Port 5" },
	{ 3, 0x2C, "Comm Port 6", "Comm Port 6" },
	{ 3, 0x2E, "Comm Port 7", "Comm Port 7" },
	{ 3, 0x30, "PWM control", "PWM control" },
	{ 3, 0x32, "Cycle register", "Cycle register" },
	{ 3, 0x34, "L ch Pulse Width", "L ch Pulse Width" },
	{ 3, 0x36, "R ch Pulse Width", "R ch Pulse Width" },
	{ 3, 0x38, "Mono Pulse Width", "Mono Pulse Width" },
	{ 2, 0x100, "", "Bitmap mode" },
	{ 2, 0x102, "", "Packed Pixel Control" },
	{ 2, 0x104, "", "Auto fill length" },
	{ 2, 0x106, "", "Auto fill start" },
	{ 2, 0x108, "", "Auto fill data" },
	{ 2, 0x10A, "", "Frame Buffer control" },
	{ 0xff, -1, "" }
};




void _32X_RegInit_KMod(HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i, adr;
	char		buf[64];
	TCHAR       szString[6][20] = { "Description", "MD Address", "Value", "Value", "32X Address", "Description" };

	h32XRegList = GetDlgItem(hwnd, IDC_32XREG_LIST);
	ListView_DeleteAllItems(h32XRegList);

	//	GetWindowRect( h32XRegList, &rSize);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 140;
	lvColumn.pszText = szString[0];
	ListView_InsertColumn(h32XRegList, 0, &lvColumn);

	lvColumn.cx = 80;
	lvColumn.pszText = szString[1];
	ListView_InsertColumn(h32XRegList, 1, &lvColumn);

	lvColumn.cx = 60;
	lvColumn.pszText = szString[2];
	ListView_InsertColumn(h32XRegList, 2, &lvColumn);

	lvColumn.cx = 60;
	lvColumn.pszText = szString[3];
	ListView_InsertColumn(h32XRegList, 3, &lvColumn);

	lvColumn.cx = 80;
	lvColumn.pszText = szString[4];
	ListView_InsertColumn(h32XRegList, 4, &lvColumn);

	lvColumn.cx = 140;
	lvColumn.pszText = szString[5];
	ListView_InsertColumn(h32XRegList, 5, &lvColumn);

	ListView_SetExtendedListViewStyle(h32XRegList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);


	i = 0;
	while (_32X_register[i].side != 0xff)
	{

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = _32X_register[i].descriptionMD;
		ListView_InsertItem(h32XRegList, &lvItem);


		if (_32X_register[i].side & 1)
		{
			adr = 0xA15100;
			adr |= _32X_register[i].adr;

			lvItem.iSubItem = 1;
			wsprintf(buf, "0x%0.6X", adr);
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);

		}

		if (_32X_register[i].side & 2)
		{
			adr = 0x4000;
			adr |= _32X_register[i].adr;

			lvItem.iSubItem = 4;
			wsprintf(buf, "0x%0.6X", adr);
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);

		}


		lvItem.iSubItem = 5;
		lvItem.pszText = _32X_register[i].description32X;
		ListView_SetItem(h32XRegList, &lvItem);

		i++;

	}
	ListView_Scroll(h32XRegList, 100, 0);
}

void Update32X_Reg_KMod()
{
	int         i;
	char		buf[64];
	LVITEM		lvItem;


	lvItem.mask = LVIF_TEXT;


	i = 0;
	while (_32X_register[i].side != 0xff)
	{
		lvItem.iItem = i;
		if (_32X_register[i].side & 1)
		{
			wsprintf(buf, "0x%0.4X", M68K_RW((0xA15100 | _32X_register[i].adr)));
			lvItem.iSubItem = 2;
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		}

		if (_32X_register[i].side & 2)
		{
			wsprintf(buf, "0x%0.4X", SH2_Read_Word(&M_SH2, (0x4000 | _32X_register[i].adr)));
			lvItem.iSubItem = 3;
			lvItem.pszText = buf;
			ListView_SetItem(h32XRegList, &lvItem);
		}

		i++;

	}
}


BOOL CALLBACK _32X_RegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_INITDIALOG:
		s32xreg_reset();
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_32_REG);
		break;

	case WM_DESTROY:
		s32xreg_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


void s32xreg_create(HINSTANCE hInstance, HWND hWndParent)
{
	h32X_Reg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG32X_REG), hWndParent, _32X_RegDlgProc);
}

void s32xreg_show(BOOL visibility)
{
	ShowWindow(h32X_Reg, visibility ? SW_SHOW : SW_HIDE);
}

void s32xreg_update()
{
	if (OpenedWindow_KMod[DMODE_32_REG - 1] == FALSE)	return;

	Update32X_Reg_KMod();
}

void s32xreg_reset()
{
	_32X_RegInit_KMod(h32X_Reg);
}
void s32xreg_destroy()
{
	DestroyWindow(h32X_Reg);
}
