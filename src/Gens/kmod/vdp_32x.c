#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_32X.h"
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "vdp_32x.h"

static HWND h32X_VDP;
static long palH, palV;

void Update32X_VDP_KMod()
{
	RedrawWindow(GetDlgItem(h32X_VDP, IDC_32XVDP_TILES), NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(h32X_VDP, IDC_32XVDP_TILES2), NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(h32X_VDP, IDC_32XVDP_PAL), NULL, NULL, RDW_INVALIDATE);

}

void Draw32XPal_KMod(LPDRAWITEMSTRUCT hlDIS)
{

	unsigned char i, j, tone;
	HBRUSH newBrush = NULL;
	HPEN hPen, hPenOld;
	RECT rc;
	LONG pix, h;
	COLORREF col;

	palV = (hlDIS->rcItem.bottom - hlDIS->rcItem.top) / 64;
	palH = (hlDIS->rcItem.right - hlDIS->rcItem.left) / 4;

	for (j = 0; j < 64; j++)
	{
		rc.top = j* palV;
		rc.bottom = rc.top + palV;

		for (i = 0; i < 4; i++)
		{
			if (newBrush)	DeleteObject((HGDIOBJ)newBrush);
			rc.left = i*palH;
			rc.right = rc.left + palH;

			// COLORREF = 0x00bbggrr
			// pix = bgr (3*5bit)
			pix = _32X_VDP_CRam[i + j * 4];
			col = 0x000000;
			tone = (pix >> 10) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = (pix >> 5) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = pix & 0x1F;
			col |= (tone << 3);

			newBrush = CreateSolidBrush(col);
			FillRect(hlDIS->hDC, &rc, newBrush);

			if (pix & 0x80)
			{
				//FillRect(hlDIS->hDC, &rc, (HBRUSH) (COLOR_WINDOWFRAME+1));

				hPen = CreatePen(PS_DOT, 1, col);
				hPenOld = SelectObject(hlDIS->hDC, hPen);

				for (h = rc.top; h < rc.bottom; h += 4)
				{
					MoveToEx(hlDIS->hDC, rc.left, h, NULL);
					LineTo(hlDIS->hDC, rc.right, h);
					MoveToEx(hlDIS->hDC, rc.left + 2, h + 2, NULL);
					LineTo(hlDIS->hDC, rc.right, h + 2);
				}

				SelectObject(hlDIS->hDC, hPenOld);
				DeleteObject(hPen);

			}

		}
	}

	if (newBrush)	DeleteObject((HGDIOBJ)newBrush);



}

void Draw32XVDP_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	unsigned char toDraw, tone;
	WORD pix, lineToDraw;
	WORD *VRAM, *lineTable;
	COLORREF col;
	WORD posX, posY, maxY, maxX;

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);


	toDraw = (unsigned char)_32X_VDP.State & 1;
	if (IsDlgButtonChecked(h32X_VDP, IDC_32XVDP_FB2) == BST_CHECKED)
	{
		toDraw = 0;
	}
	else if (IsDlgButtonChecked(h32X_VDP, IDC_32XVDP_FB2) == BST_CHECKED)
	{
		toDraw = 1;
	}

	lineTable = (WORD *)(_32X_VDP_Ram + ((unsigned int)toDraw * 0x20000));

	maxX = 320;
	maxY = 240;

	for (posY = 0; posY < maxY; posY++)
	{

		lineToDraw = lineTable[posY];
		VRAM = lineTable + lineToDraw;
		for (posX = 0; posX < maxX; posX++)
		{
			// COLORREF = 0x00bbggrr
			// pix = bgr (3*5bit)
			pix = VRAM[posX];
			col = 0x000000;
			tone = (pix >> 10) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = (pix >> 5) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = pix & 0x1F;
			col |= (tone << 3);


			SetPixelV(hDC, posX, posY, col);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		0,  // y-coordinate of destination rectangle's upper-left
		// corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left
		// corner
		0,   // y-coordinate of source rectangle's upper-left
		// corner
		SRCCOPY  // raster operation code
		);

	SelectObject(hDC, hOldBitmap);
	DeleteObject(hBitmap);

	DeleteDC(hDC);
}

void Draw32XVDPRaw_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	unsigned char toDraw, tone;
	WORD pix;
	WORD *VRAM;
	COLORREF col;
	WORD posX, posY, maxY, maxX;

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);


	toDraw = (unsigned char)_32X_VDP.State & 1;
	if (IsDlgButtonChecked(h32X_VDP, IDC_32XVDP_FB2) == BST_CHECKED)
	{
		toDraw = 0;
	}
	else if (IsDlgButtonChecked(h32X_VDP, IDC_32XVDP_FB2) == BST_CHECKED)
	{
		toDraw = 1;
	}

	VRAM = (WORD *)(_32X_VDP_Ram + ((unsigned int)toDraw * 0x20000) + 256 * 2); //skip 256 words of line table


	maxX = 320;
	maxY = 240;

	for (posY = 0; posY < maxY; posY++)
	{
		for (posX = 0; posX < maxX; posX++)
		{
			// COLORREF = 0x00bbggrr
			// pix = bgr (3*5bit)
			pix = VRAM[posX + (posY*maxX)];
			col = 0x000000;
			tone = (pix >> 10) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = (pix >> 5) & 0x1F;
			col |= (tone << 3);
			col <<= 8;
			tone = pix & 0x1F;
			col |= (tone << 3);


			SetPixelV(hDC, posX, posY, col);

		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		0,  // y-coordinate of destination rectangle's upper-left
		// corner
		maxX,  // width of destination rectangle
		maxY, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left
		// corner
		0,   // y-coordinate of source rectangle's upper-left
		// corner
		SRCCOPY  // raster operation code
		);

	SelectObject(hDC, hOldBitmap);
	DeleteObject(hBitmap);

	DeleteDC(hDC);
}


void Dump32XCRAM_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_CRAM");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "32X CRAM dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 32X CRAM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 0x100;
	WriteFile(hFr, _32X_VDP_CRam_Ajusted, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("32X CRAM dumped", 1500);
}

void Dump32XVDP_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VDP");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "Frame Buffer dump (*.bin)\0*.bin\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump 32X Frame buffer";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bin";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 0x100 * 1024;
	WriteFile(hFr, _32X_VDP_Ram, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("32X Frame buffer dumped", 1500);
}


BOOL CALLBACK _32X_VDPDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	switch (Message)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hwnd, IDC_32XVDP_FB0, IDC_32XVDP_FB2, IDC_32XVDP_FB0);
		break;

	case WM_DRAWITEM:
		if ((UINT)wParam == IDC_32XVDP_TILES)
			Draw32XVDPRaw_KMod((LPDRAWITEMSTRUCT)lParam);
		else if ((UINT)wParam == IDC_32XVDP_TILES2)
			Draw32XVDP_KMod((LPDRAWITEMSTRUCT)lParam);
		else if ((UINT)wParam == IDC_32XVDP_PAL)
			Draw32XPal_KMod((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_32XVDP_DUMP:
			Dump32XVDP_KMod(h32X_VDP);
			break;
		case IDC_32XVDP_CRAM:
			Dump32XCRAM_KMod(h32X_VDP);
			break;
		case IDC_32XVDP_FB0:
		case IDC_32XVDP_FB1:
		case IDC_32XVDP_FB2:
			Update32X_VDP_KMod();
			break;
		}
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_32_VDP);
		break;

	case WM_DESTROY:
		vdp32x_destroy();
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


void vdp32x_create(HINSTANCE hInstance, HWND hWndParent)
{
	h32X_VDP = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUG32X_VDP), hWndParent, _32X_VDPDlgProc);
}

void vdp32x_show(BOOL visibility)
{
	ShowWindow(h32X_VDP, visibility ? SW_SHOW : SW_HIDE);
}

void vdp32x_update()
{
	if (OpenedWindow_KMod[DMODE_32_VDP-1] == FALSE)	return;

	Update32X_VDP_KMod();
}

void vdp32x_reset()
{

}
void vdp32x_destroy()
{
	DestroyWindow(h32X_VDP);
}

