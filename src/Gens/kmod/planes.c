#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_io.h"

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "planes.h"

HWND hPlaneExplorer;

static unsigned char plane_explorer_data[128 * 8 * 128 * 8];
static COLORREF plane_explorer_palette[256];
static int old_plane_width = 0;
static int old_plane_height = 0;
static int plane_explorer_plane = 0;
static BOOL show_transparence = FALSE;


static void PlaneExplorerInit_KMod(HWND hDlg)
{
	HWND hexplorer;
	RECT rc;

	SendDlgItemMessage(hDlg, IDC_PLANEEXPLORER_COMBO, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	SendDlgItemMessage(hDlg, IDC_PLANEEXPLORER_COMBO, CB_INSERTSTRING, (WPARAM)-1, (LONG)(LPTSTR) "Plane A");
	SendDlgItemMessage(hDlg, IDC_PLANEEXPLORER_COMBO, CB_INSERTSTRING, (WPARAM)-1, (LONG)(LPTSTR) "Plane B");
	
	//TODO add support for 32X ?

	SendDlgItemMessage(hDlg, IDC_PLANEEXPLORER_COMBO, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	InitCommonControls();


	hexplorer = (HWND)GetDlgItem(hDlg, IDC_PLANEXPLEORER_MAIN);
	GetClientRect(hDlg, &rc);
	MoveWindow(hexplorer, 20, 60, (rc.right - rc.left) - 40, (rc.bottom - rc.top) - 80, TRUE);
}

static void PlaneExplorer_UpdatePalette(void)
{
	COLORREF * cr = &plane_explorer_palette[0];
	COLORREF col;
	unsigned short * pal = (unsigned short *)(&CRam[0]);
	int i;
	static const COLORREF normal_pal[] =
	{
		0x00000000,
		0x00000011,
		0x00000022,
		0x00000033,
		0x00000044,
		0x00000055,
		0x00000066,
		0x00000077,
		0x00000088,
		0x00000099,
		0x000000AA,
		0x000000BB,
		0x000000CC,
		0x000000DD,
		0x000000EE,
		0x000000FF
	};

	for (i = 0; i < 64; i++)
	{
		unsigned short p = *pal++;
		col = normal_pal[(p >> 8) & 0xF] << 0;
		col |= normal_pal[(p >> 4) & 0xF] << 8;
		col |= normal_pal[(p >> 0) & 0xF] << 16;
		*cr++ = col;
	}

	plane_explorer_palette[253] = 0x00333333;
	plane_explorer_palette[254] = 0x00444444;
	plane_explorer_palette[255] = 0x00555555;
}

union PATTERN_NAME
{
	struct
	{
		unsigned short tile_index : 11;
		unsigned short h_flip : 1;
		unsigned short v_flip : 1;
		unsigned short pal_index : 2;
		unsigned short priority : 1;
	};
	unsigned short word;
};

static unsigned short byte_swap(unsigned short w)
{
	return (w >> 8) | (w << 8);
}

static void PlaneExplorer_DrawTile(unsigned short name_word, unsigned int x, unsigned int y, int transcolor)
{
	union PATTERN_NAME name;
	unsigned int tile_height = ((VDP_Reg.Set4 & 0x6) == 6) ? 16 : 8;
	unsigned char * ptr = &plane_explorer_data[y * 1024 * tile_height + x * 8];
	unsigned int j, k;
	unsigned int * tile_data;
	int stride = 1024;
	unsigned char pal_index;

	name.word = name_word;
	tile_data = (unsigned int *)(&VRam[(name.tile_index * tile_height * 4) & 0xFFFF]);
	pal_index = (unsigned char)name.pal_index << 4;

	if (name.v_flip)
	{
		ptr += (tile_height - 1) * stride;
		stride = -stride;
	}

	if (name.h_flip)
	{
		for (j = 0; j < tile_height; j++)
		{
			unsigned int tile_row = tile_data[j];
			ptr[4] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[5] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[6] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[7] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[0] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[1] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[2] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[3] = (tile_row & 0xF) | pal_index;
			if (transcolor != -1)
			{
				for (k = 0; k < 8; k++)
				{
					if (ptr[k] == pal_index)
					{
						ptr[k] = transcolor;
					}
				}
			}
			ptr += stride;
		}
	}
	else
	{
		for (j = 0; j < tile_height; j++)
		{
			unsigned int tile_row = tile_data[j];
			ptr[3] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[2] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[1] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[0] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[7] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[6] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[5] = (tile_row & 0xF) | pal_index;    tile_row >>= 4;
			ptr[4] = (tile_row & 0xF) | pal_index;
			if (transcolor != -1)
			{
				for (k = 0; k < 8; k++)
				{
					if (ptr[k] == pal_index)
					{
						ptr[k] = transcolor;
					}
				}
			}
			ptr += stride;
		}
	}
}

static void PlaneExplorer_UpdateBitmap(HWND hwnd, int plane)
{
	unsigned int i, j;

	unsigned int plane_width = 32 + (VDP_Reg.Scr_Size & 0x3) * 32;
	unsigned int plane_height = 32 + ((VDP_Reg.Scr_Size >> 4) & 0x3) * 32;
	unsigned int plane_a_base = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
	unsigned int plane_b_base = (VDP_Reg.Pat_ScrB_Adr & 0x7) << 13;
	unsigned short * plane_a = (unsigned short *)(&VRam[plane_a_base]);
	unsigned short * plane_b = (unsigned short *)(&VRam[plane_b_base]);
	unsigned short * plane_data = (plane == 0) ? plane_a : plane_b;

	if (plane_width != old_plane_width ||
		plane_height != old_plane_height)
	{
		old_plane_width = plane_width;
		old_plane_height = plane_height;
		for (i = 0; i < 1024; i++)
		{
			for (j = 0; j < 1024; j++)
			{
				plane_explorer_data[i * 1024 + j] = (unsigned char)(((j ^ i) >> 2) & 1) + 253;
			}
		}
	}

	for (j = 0; j < plane_height; j++)
	{
		for (i = 0; i < plane_width; i++)
		{
			int trans_color = show_transparence ? (unsigned char)(((j ^ i) >> 1) & 1) + 254 : -1;
			PlaneExplorer_DrawTile(plane_data[j * plane_width + i], i, j, trans_color);
		}
	}
}

static void PlaneExplorerPaint_KMod(HWND hwnd, LPDRAWITEMSTRUCT lpdi)
{
	struct BMI_LOCAL
	{
		BITMAPINFOHEADER hdr;
		COLORREF         palette[256];
	};

	struct BMI_LOCAL bmi =
	{
		{
			sizeof(BITMAPINFOHEADER),
			128 * 8,
			-128 * 8,
			1,
			8,
			BI_RGB,
			0,
			250,
			250,
			256,
			256,
		},
		{
			0x00FF0055,
		}
	};

	unsigned int plane_a_base = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
	unsigned int plane_b_base = (VDP_Reg.Pat_ScrB_Adr & 0x7) << 13;

	char buffer[1024];
	int plane;

	PlaneExplorer_UpdatePalette();

	plane = (int)SendDlgItemMessage(hwnd, IDC_PLANEEXPLORER_COMBO, CB_GETCURSEL, 0, 0);

	PlaneExplorer_UpdateBitmap(hwnd, plane);

	memcpy(bmi.palette, plane_explorer_palette, sizeof(bmi.palette));

	SetDIBitsToDevice(
		lpdi->hDC,
		lpdi->rcItem.left, lpdi->rcItem.top,
		lpdi->rcItem.right - lpdi->rcItem.left,
		lpdi->rcItem.bottom - lpdi->rcItem.top,
		lpdi->rcItem.left, lpdi->rcItem.top,
		lpdi->rcItem.top, lpdi->rcItem.bottom - lpdi->rcItem.top,
		plane_explorer_data,
		(const BITMAPINFO *)&bmi,
		DIB_RGB_COLORS);

	{
		static unsigned int old_scr_size = 0xFFF;
		static unsigned int old_a_base = 0;
		static unsigned int old_b_base = 0;
		static unsigned int old_mode = 0xFFF;

		if (old_scr_size != VDP_Reg.Scr_Size ||
			old_a_base != plane_a_base ||
			old_b_base != plane_b_base)
		{
			old_mode = VDP_Reg.Set4 & 0x6;
			wsprintf(buffer, "Width: %d Height %d: Plane A Base: 0x%04X Plane B Base: 0x%04X: Mode %s",
				32 + (VDP_Reg.Scr_Size & 0x3) * 32,
				32 + ((VDP_Reg.Scr_Size >> 4) & 0x3) * 32,
				plane_a_base, plane_b_base,
				(old_mode == 2) ? "Interlaced" :
				(old_mode == 6) ? "Double interlaced" :
				"Normal");

			SetDlgItemText(hwnd, IDC_PLANEEXPLORER_PROPS, buffer);

			old_scr_size = VDP_Reg.Scr_Size;
			old_a_base = plane_a_base;
			old_b_base = plane_b_base;
		}
	}
}

void PlaneExplorer_GetTipText(int x, int y, char * buffer)
{
	int plane_size_x = 32 + (VDP_Reg.Scr_Size & 0x3) * 32;
	int plane_size_y = 32 + ((VDP_Reg.Scr_Size >> 4) & 0x3) * 32;


	unsigned int plane_a_base = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
	unsigned int plane_b_base = (VDP_Reg.Pat_ScrB_Adr & 0x7) << 13;
	unsigned int base = plane_explorer_plane ? plane_b_base : plane_a_base;
	char plane_char = plane_explorer_plane ? 'B' : 'A';
	unsigned int tile_addr = base + ((y >> 3) * plane_size_x + (x >> 3)) * 2;
	union PATTERN_NAME name;


	if (x >= (plane_size_x * 8) ||
		y >= (plane_size_y * 8))
	{
		buffer[0] = 0;
		return;
	}

	name.word = *(unsigned short *)(&VRam[tile_addr]);

	wsprintf(buffer, "%d, %d in plane %c @ 0x%04X is @ 0x%04X = 0x%04X [tile %d, pal %d,%s%s prior %d]",
		x >> 3,
		y >> 3,
		plane_char,
		base,
		tile_addr,
		name.word,
		name.tile_index,
		name.pal_index,
		name.h_flip ? " HFLIP," : "",
		name.v_flip ? " VFLIP," : "",
		name.priority
		);
}

BOOL CALLBACK PlaneExplorerDialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		PlaneExplorerInit_KMod(hwnd);
		break;

	case WM_DRAWITEM:
		PlaneExplorerPaint_KMod(hwnd, (LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PLANEEXPLORER_COMBO:
			plane_explorer_plane = (int)SendDlgItemMessage(hwnd, IDC_PLANEEXPLORER_COMBO, CB_GETCURSEL, 0, 0);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case IDC_PLANEEXPLORER_TRANS:
			show_transparence = (IsDlgButtonChecked(hwnd, IDC_PLANEEXPLORER_TRANS) == BST_CHECKED);
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		default:
			break;
		}
		break;

	case WM_SIZE:
	{
		HWND hexplorer = GetDlgItem(hwnd, IDC_PLANEXPLEORER_MAIN);
		MoveWindow(hexplorer, 20, 60, LOWORD(lParam) - 40, HIWORD(lParam) - 80, TRUE);
		break;
	}

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_PLANEEXPLORER);
		break;

	case WM_DESTROY:
		planes_destroy();
		PostQuitMessage(0);
		break;

	case WM_MOUSELEAVE:
		SetDlgItemText(hwnd, IDC_PLANEEXPLORER_TILEINFO, "TILE: ");
		return FALSE;

	case WM_MOUSEMOVE:
	{
		HWND hexplorer = GetDlgItem(hwnd, IDC_PLANEXPLEORER_MAIN);
		int x = (short)(lParam);
		int y = (short)(lParam >> 16);
		char buffer[180] = "TILE: ";
		RECT rc1;
		POINT pt;
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.hwndTrack = hwnd;
		tme.dwFlags = TME_LEAVE;
		TrackMouseEvent(&tme);
		pt.x = x;
		pt.y = y;
		ClientToScreen(hwnd, &pt);
		ScreenToClient(hexplorer, &pt);
		GetClientRect(hexplorer, &rc1);
		if (PtInRect(&rc1, pt))
		{
			PlaneExplorer_GetTipText(pt.x, pt.y, buffer + 6);
		}
		SetDlgItemText(hwnd, IDC_PLANEEXPLORER_TILEINFO, buffer);
		return FALSE;
	}

	default:
		return FALSE;
	}

	return TRUE;
}

void planes_create(HINSTANCE hInstance, HWND hWndParent)
{
	hPlaneExplorer = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGPLANEEXPLORER), hWndParent, PlaneExplorerDialogProc);
}

void planes_show(BOOL visibility)
{
	ShowWindow(hPlaneExplorer, visibility ? SW_SHOW : SW_HIDE);
}

void planes_update()
{
	if (OpenedWindow_KMod[18] == FALSE)	return;

	RedrawWindow(GetDlgItem(hPlaneExplorer, IDC_PLANEXPLEORER_MAIN), NULL, NULL, RDW_INVALIDATE);
}

void planes_reset()
{
	PlaneExplorerInit_KMod(hPlaneExplorer);
}
void planes_destroy()
{
	DestroyWindow(hPlaneExplorer);
}