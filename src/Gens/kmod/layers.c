#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_rend.h" //for ActiveLayer

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "layers.h"

HWND hLayers;


BOOL CALLBACK LayersDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		layers_reset();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LAYER_A:
			if (IsDlgButtonChecked(hwnd, IDC_LAYER_A) == BST_CHECKED)
				ActiveLayer |= 0x08;
			else
				ActiveLayer &= 0x17;
			break;
		case IDC_LAYER_B:
			if (IsDlgButtonChecked(hwnd, IDC_LAYER_B) == BST_CHECKED)
				ActiveLayer |= 0x04;
			else
				ActiveLayer &= 0x1B;
			break;
		case IDC_LAYER_SPRITE:
			if (IsDlgButtonChecked(hwnd, IDC_LAYER_SPRITE) == BST_CHECKED)
				ActiveLayer |= 0x02;
			else
				ActiveLayer &= 0x1D;
			break;
		case IDC_LAYER_WINDOW:
			if (IsDlgButtonChecked(hwnd, IDC_LAYER_WINDOW) == BST_CHECKED)
				ActiveLayer |= 0x01;
			else
				ActiveLayer &= 0x1E;
			break;
		case IDC_LAYER_32X:
			if (IsDlgButtonChecked(hwnd, IDC_LAYER_32X) == BST_CHECKED)
				ActiveLayer |= 0x10;
			else
				ActiveLayer &= 0x0F;
			break;
		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_LAYERS);
		break;

	case WM_DESTROY:
		layers_destroy();

		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void layers_create(HINSTANCE hInstance, HWND hWndParent)
{
	hLayers = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_LAYERS), hWndParent, LayersDlgProc);
}

void layers_show(BOOL visibility)
{
	ShowWindow(hLayers, visibility ? SW_SHOW : SW_HIDE);
}

void layers_update()
{

}

void layers_reset()
{
	CheckDlgButton(hLayers, IDC_LAYER_A, BST_CHECKED);
	CheckDlgButton(hLayers, IDC_LAYER_B, BST_CHECKED);
	CheckDlgButton(hLayers, IDC_LAYER_SPRITE, BST_CHECKED);
	CheckDlgButton(hLayers, IDC_LAYER_WINDOW, BST_CHECKED);
	CheckDlgButton(hLayers, IDC_LAYER_32X, BST_CHECKED);
	ActiveLayer = 0x1F;
}
void layers_destroy()
{
	DestroyWindow(hLayers);
}