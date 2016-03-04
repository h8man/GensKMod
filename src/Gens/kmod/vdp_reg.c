#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h"
#include "../resource.h"
#include "../vdp_io.h"
#include "../vdp_rend.h"

#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "vdp_reg.h"

static HWND hRegister;
static HWND hVDPReg_List;
static CHAR debug_string[1024];

void VDPRegInit_KMod(HWND hwnd)
{
	LV_COLUMN   lvColumn;
	LVITEM		lvItem;
	int         i;
	char		buf[64];

	hVDPReg_List = GetDlgItem(hwnd, IDC_VDP_REG_LIST);
	ListView_DeleteAllItems(hVDPReg_List);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 40;
	lvColumn.pszText = "Reg";
	ListView_InsertColumn(hVDPReg_List, 0, &lvColumn);

	lvColumn.cx = 40;
	lvColumn.pszText = "Value";
	ListView_InsertColumn(hVDPReg_List, 1, &lvColumn);

	lvColumn.cx = 280;
	lvColumn.pszText = "Desc";
	ListView_InsertColumn(hVDPReg_List, 2, &lvColumn);

	ListView_SetExtendedListViewStyle(hVDPReg_List, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	for (i = 0; i <= 23; i++)
	{
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0;
		wsprintf(buf, "#%0.2d", i);
		lvItem.pszText = buf;
		lvItem.iItem = i;
		ListView_InsertItem(hVDPReg_List, &lvItem);
		/*
		lvItem.iSubItem = 1;
		lvItem.pszText = "";
		ListView_SetItem(hVDPReg_List, &lvItem);

		lvItem.iSubItem = 2;
		lvItem.pszText = "";
		ListView_SetItem(hVDPReg_List, &lvItem);
		*/
	}
}

void UpdateVDPReg_KMod()
{
	BYTE		valReg;
	LVITEM		lvItem;
	static struct Reg_VDP_Type old_regs = { 0, };

	if (!memcmp(&old_regs, &VDP_Reg, sizeof(old_regs)))
		return;

	memcpy(&old_regs, &VDP_Reg, sizeof(old_regs));

	lvItem.mask = LVIF_TEXT;

	// Reg 0
	valReg = VDP_Reg.Set1;
	lvItem.iItem = 0;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "");
	if (valReg & 0x10)	lstrcat(debug_string, "HIntEnable ");
	if (valReg & 0x02)	lstrcat(debug_string, "HV counter stop ");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 1
	valReg = VDP_Reg.Set2;
	lvItem.iItem = 1;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "");
	if (valReg & 0x40)	lstrcat(debug_string, "DisplayEnable ");
	if (valReg & 0x20)	lstrcat(debug_string, "VintEnable ");
	if (valReg & 0x10)	lstrcat(debug_string, "DMAEnable ");

	if (valReg & 0x08)
		lstrcat(debug_string, "V30");
	else
		lstrcat(debug_string, "V28");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);



	// Reg 2
	valReg = VDP_Reg.Pat_ScrA_Adr;
	lvItem.iItem = 2;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "FieldA: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 3
	valReg = VDP_Reg.Pat_Win_Adr;
	lvItem.iItem = 3;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 4
	valReg = VDP_Reg.Pat_ScrB_Adr;
	lvItem.iItem = 4;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "FieldB: 0x%.2X00", valReg << 5);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 5
	valReg = VDP_Reg.Spr_Att_Adr;
	lvItem.iItem = 5;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Sprite: 0x%.2X00", valReg << 1);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 6
	valReg = VDP_Reg.Reg6;
	lvItem.iItem = 6;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 7
	valReg = VDP_Reg.BG_Color;
	lvItem.iItem = 7;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "BG Color (Pal %d Idx %d)", valReg / 16, valReg % 16);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 8
	valReg = VDP_Reg.Reg8;
	lvItem.iItem = 8;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 9
	valReg = VDP_Reg.Reg9;
	lvItem.iItem = 9;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 10
	valReg = VDP_Reg.H_Int;
	lvItem.iItem = 10;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "HInt value : %d", valReg);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 11
	valReg = VDP_Reg.Set3;
	lvItem.iItem = 11;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	debug_string[0] = 0;
	if (valReg & 0x08)	lstrcat(debug_string, "EIntEnable ");

	if (valReg & 0x04)
		lstrcat(debug_string, "Half VScroll ");
	else
		lstrcat(debug_string, "Full VScroll ");

	switch (valReg & 0x03)
	{
	case 0:
		lstrcat(debug_string, "Full HScroll");
		break;
	case 1:
		break;
	case 2:
		lstrcat(debug_string, "Cell HScroll");
		break;
	case 3:
		lstrcat(debug_string, "Line HScroll");
		break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 12
	valReg = VDP_Reg.Set4;
	lvItem.iItem = 12;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	debug_string[0] = 0;
	if (valReg & 0x08) lstrcat(debug_string, "Shadow/Highlight ");

	switch (valReg & 0x06)
	{
	case 0:
		break;
	case 2:
		lstrcat(debug_string, "Interlace ");
		break;
	case 4:
		break;
	case 6:
		lstrcat(debug_string, "Double Interlace ");
		break;
	}
	if (valReg & 0x01)
		lstrcat(debug_string, "H40");
	else
		lstrcat(debug_string, "H32");

	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 13
	valReg = VDP_Reg.H_Scr_Adr;
	lvItem.iItem = 13;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "HScroll: 0x%.2X00", valReg << 2);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 14
	valReg = VDP_Reg.Reg14;
	lvItem.iItem = 14;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 15
	valReg = VDP_Reg.Auto_Inc;
	lvItem.iItem = 15;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Auto Increment of %d", valReg);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 16
	valReg = VDP_Reg.Scr_Size;
	lvItem.iItem = 16;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	debug_string[0] = 0;
	switch (valReg & 0x30)
	{
	case 0x00:
		lstrcat(debug_string, "V32 Cell ");
		break;
	case 0x10:
		lstrcat(debug_string, "V64 Cell ");
		break;
	case 0x20:
		break;
	case 0x30:
		lstrcat(debug_string, " V128 Cell");
		break;
	}
	switch (valReg & 0x03)
	{
	case 0x00:
		lstrcat(debug_string, "H32 Cell");
		break;
	case 0x01:
		lstrcat(debug_string, "H64 Cell");
		break;
	case 0x02:
		break;
	case 0x03:
		lstrcat(debug_string, "H128 Cell");
		break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 17
	valReg = VDP_Reg.Win_H_Pos;
	lvItem.iItem = 17;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window is %d cells ", valReg & 0x1F);
	if (valReg & 0x80)
		lstrcat(debug_string, "from right");
	else
		lstrcat(debug_string, "from left");
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 18
	valReg = VDP_Reg.Win_V_Pos;
	lvItem.iItem = 18;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "Window is %d cells ", valReg & 0x1F);
	if (valReg & 0x80)
		lstrcat(debug_string, "from top");
	else
		lstrcat(debug_string, "from bottom");
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 19
	valReg = VDP_Reg.DMA_Lenght_L;
	lvItem.iItem = 19;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 20
	valReg = VDP_Reg.DMA_Lenght_H;
	lvItem.iItem = 20;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "DMA Length 0x%0.2X%0.2X", valReg, VDP_Reg.DMA_Lenght_L);
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 21
	valReg = VDP_Reg.DMA_Src_Adr_L;
	lvItem.iItem = 21;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 22
	valReg = VDP_Reg.DMA_Src_Adr_M;
	lvItem.iItem = 22;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	lvItem.iSubItem = 2;
	lvItem.pszText = "see below";
	ListView_SetItem(hVDPReg_List, &lvItem);

	// Reg 23
	valReg = VDP_Reg.DMA_Src_Adr_H;
	lvItem.iItem = 23;
	lvItem.iSubItem = 1;
	wsprintf(debug_string, "0x%0.2X", valReg);
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);

	wsprintf(debug_string, "DMA Source 0x00%0.2X%0.2X%0.2X", valReg & 0x3F, VDP_Reg.DMA_Src_Adr_M, VDP_Reg.DMA_Src_Adr_L);
	valReg >>= 6;
	lstrcat(debug_string, ", ");
	switch (valReg)
	{
	case 0x00:
	case 0x01:
		lstrcat(debug_string, "Memory copy");
		break;
	case 0x02:
		lstrcat(debug_string, "VRAM Fill");
		break;
	case 0x03:
		lstrcat(debug_string, "VRAM Copy");
		break;
	}
	lvItem.iSubItem = 2;
	lvItem.pszText = debug_string;
	ListView_SetItem(hVDPReg_List, &lvItem);
}



BOOL CALLBACK VDPRegDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		vdpreg_reset();
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_VDP_REG);
		break;
	case WM_DESTROY:
		vdpreg_destroy();

		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}


void vdpreg_create(HINSTANCE hInstance, HWND hWndParent)
{
	hRegister = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGVDP_REG), hWndParent, VDPRegDlgProc);
}

void vdpreg_show(BOOL visibility)
{
	ShowWindow(hRegister, visibility ? SW_SHOW : SW_HIDE);
}

void vdpreg_update()
{
	if (OpenedWindow_KMod[DMODE_VDP_REG - 1] == FALSE)	return;

	UpdateVDPReg_KMod();
}

void vdpreg_reset()
{
	VDPRegInit_KMod(hRegister);
}
void vdpreg_destroy()
{
	DestroyWindow(hRegister);
}
