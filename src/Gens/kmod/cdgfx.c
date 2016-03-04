#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../G_main.h" //for Paused
#include "../resource.h"
#include "../Mem_S68k.h"
#include "../G_gfx.h" //used for Put_Info

//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "cdgfx.h"
//// size WRAM = 256*1024....4096tiles (64b) ==> (32*8)*(64*16)

#include "vdp.h"
//TODO what the exact use of pal_Kmod here ? why not CDpal_KMod ?

static HWND hCD_GFX;
static UCHAR CDpal_KMod, CDtileBank_KMod, CDzoomTile_KMod, CDTileCurDraw;
static CHAR debug_string[1024];





void DumpTilesCD_KMod(HWND hwnd)
{

	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits;

	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	WORD posX, posY;
	WORD maxX, maxY;

	COLORREF		tmpColor;
	DWORD			j;

	unsigned short int numTile = 0;
	unsigned char		TileData;


	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VRAM");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Convert Word RAM to bitmap";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bmp";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + 64 * 2000 / 2); // 4bits per pixel
	bmfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));


	maxY = 64 * 16;
	maxX = 32 * 8;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = maxX;
	bmiHeader.biHeight = maxY;
	bmiHeader.biPlanes = 0;
	bmiHeader.biBitCount = 4;
	bmiHeader.biClrUsed = 16;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = (bmiHeader.biWidth*bmiHeader.biHeight) / 2;
	bmiHeader.biClrImportant = 0;

	for (j = 0; j < 16; j++)
	{
		tmpColor = vdpdebug_getColor(pal_KMod, j);
		bmiColors[j].rgbRed = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbGreen = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbBlue = (BYTE)tmpColor & 0xFF;
	}

	pBits = (LPBYTE)LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}

	for (posY = 0; posY < maxY; posY += 16)
	{
		for (posX = 0; posX < maxX / 2; posX += 4)
		{
			for (j = 0; j < 8; j++)
			{
				TileData = Ram_Word_2M[numTile * 64 + j * 4 + 1];
				pBits[(bmiHeader.biHeight - (posY + j))*bmiHeader.biWidth / 2 + posX + 0] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + j * 4];
				pBits[(bmiHeader.biHeight - (posY + j))*bmiHeader.biWidth / 2 + posX + 1] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + j * 4 + 3];
				pBits[(bmiHeader.biHeight - (posY + j))*bmiHeader.biWidth / 2 + posX + 2] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + j * 4 + 2];
				pBits[(bmiHeader.biHeight - (posY + j))*bmiHeader.biWidth / 2 + posX + 3] = TileData;
			}

			for (j = 0; j < 8; j++)
			{
				TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 1];
				pBits[(bmiHeader.biHeight - (posY + j + 8))*bmiHeader.biWidth / 2 + posX + 0] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4];
				pBits[(bmiHeader.biHeight - (posY + j + 8))*bmiHeader.biWidth / 2 + posX + 1] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 3];
				pBits[(bmiHeader.biHeight - (posY + j + 8))*bmiHeader.biWidth / 2 + posX + 2] = TileData;

				TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 2];
				pBits[(bmiHeader.biHeight - (posY + j + 8))*bmiHeader.biWidth / 2 + posX + 3] = TileData;
			}
			numTile++;
		}
	}

	hFr = CreateFile(szFileName, GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hFr == INVALID_HANDLE_VALUE)
	{
		LocalFree((HLOCAL)pBits);
		return;
	}

	dwBytesToWrite = sizeof(BITMAPFILEHEADER);
	WriteFile(hFr, &bmfh, dwBytesToWrite, &dwBytesWritten, NULL);

	dwBytesToWrite = sizeof(BITMAPINFOHEADER);
	WriteFile(hFr, &bmiHeader, dwBytesToWrite, &dwBytesWritten, NULL);

	dwBytesToWrite = 16 * sizeof(RGBQUAD);
	WriteFile(hFr, bmiColors, dwBytesToWrite, &dwBytesWritten, NULL);

	dwBytesToWrite = bmiHeader.biSizeImage;
	WriteFile(hFr, pBits, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	LocalFree((HLOCAL)pBits);

	Put_Info("WRAM Tiles dumped", 1500);
}


void DrawTileCD_KMod(HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom)
{
	unsigned char j;
	unsigned char TileData;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC(hDCMain);
	hBitmap = CreateCompatibleBitmap(hDCMain, 8, 16);
	hOldBitmap = SelectObject(hDC, hBitmap);


	for (j = 0; j < 8; j++)
	{
		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 1] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 0, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 1] & 0x0F;
		SetPixelV(hDC, 1, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 2, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4] & 0x0F;
		SetPixelV(hDC, 3, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 3] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 4, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 3] & 0x0F;
		SetPixelV(hDC, 5, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 2] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 6, j, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + j * 4 + 2] & 0x0F;
		SetPixelV(hDC, 7, j, vdpdebug_getColor(pal, TileData));
	}

	for (j = 0; j < 8; j++)
	{
		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 1] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 0, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 1] & 0x0F;
		SetPixelV(hDC, 1, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 2, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4] & 0x0F;
		SetPixelV(hDC, 3, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 3] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 4, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 3] & 0x0F;
		SetPixelV(hDC, 5, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 2] & 0xF0;
		TileData >>= 4;
		SetPixelV(hDC, 6, j + 8, vdpdebug_getColor(pal, TileData));

		TileData = Ram_Word_2M[numTile * 64 + 32 + j * 4 + 2] & 0x0F;
		SetPixelV(hDC, 7, j + 8, vdpdebug_getColor(pal, TileData));
	}

	StretchBlt(
		hDCMain, // handle to destination device context
		x,  // x-coordinate of destination rectangle's upper-left
		// corner
		y,  // y-coordinate of destination rectangle's upper-left
		// corner
		8 * zoom,  // width of destination rectangle
		16 * zoom, // height of destination rectangle
		hDC,  // handle to source device context
		0,   // x-coordinate of source rectangle's upper-left
		// corner
		0,   // y-coordinate of source rectangle's upper-left
		// corner
		8,
		16,
		SRCCOPY  // raster operation code
		);


	SelectObject(hDC, hOldBitmap);
	DeleteObject(hBitmap);

	DeleteDC(hDC);
}


void DrawCD_GFX_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	WORD posX, posY, maxY, maxX;
	unsigned short int numTile = 0;
	SCROLLINFO si;

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);

	//	numTile = CDtileBank_KMod*32;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo((HWND)GetDlgItem(hCD_GFX, IDC_CD_GFX_SCROLL), SB_CTL, &si);

	numTile = si.nPos * 32;

	if (Paused)
	{
		maxY = 16 * 16;
	}
	else
	{
		// speed up emulation by redrawing 1/4 each refresh
		maxY = 4 * 16;
		numTile += CDTileCurDraw * 4 * 32;
	}


	maxX = 32 * 8;
	for (posY = 0; posY < maxY; posY += 16)
	{
		for (posX = 0; posX < maxX; posX += 8)
		{
			DrawTileCD_KMod(hDC, numTile++, posX, posY, pal_KMod, 1);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		(Paused ? 0 : CDTileCurDraw*maxY),  // y-coordinate of destination rectangle's upper-left
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


void UpdateCD_GFX_KMod()
{
	if (OpenedWindow_KMod[5] == FALSE)	return;

	RedrawWindow(GetDlgItem(hCD_GFX, IDC_CD_GFX), NULL, NULL, RDW_INVALIDATE);

	CDTileCurDraw++;
	if (CDTileCurDraw > 3)	CDTileCurDraw = 0;
}


void CD_GFXInit_KMod(HWND hwnd)
{
	SCROLLINFO si;
	RECT rc;

	CDtileBank_KMod = 0;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nPage = 1;
	si.nPos = 0;
	si.nMax = 64 - 16;

	SetScrollInfo(GetDlgItem(hwnd, IDC_CD_GFX_SCROLL), SB_CTL, &si, TRUE);


	GetClientRect(GetDlgItem(hwnd, IDC_CD_GFX), &rc);
	MapWindowPoints(GetDlgItem(hwnd, IDC_CD_GFX), hwnd, (LPPOINT)&rc, 2);

	rc.bottom = rc.top + 16 * 16;
	rc.right = rc.left + 32 * 8;
	MoveWindow(GetDlgItem(hwnd, IDC_CD_GFX), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);


	MoveWindow(GetDlgItem(hwnd, IDC_CD_GFX_SCROLL), rc.right, rc.top, GetSystemMetrics(SM_CXVSCROLL), rc.bottom - rc.top, TRUE);

}


BOOL CALLBACK CD_GFXDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;

	switch (Message)
	{
	case WM_INITDIALOG:
		CD_GFXInit_KMod(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CD_GFX_DUMP:
			//TODO
			break;

		case IDC_CD_GFX_BMP:
			DumpTilesCD_KMod(hCD_GFX);
			break;
		}
		break;

	case WM_VSCROLL:
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
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
		case SB_THUMBTRACK:
			//					 si.nPos = HIWORD(wParam);
			si.nPos = si.nTrackPos;
			break;
		}

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		SetScrollInfo((HWND)lParam, SB_CTL, &si, TRUE);

		wsprintf(debug_string, "0x%.5X", (32 * si.nPos) * 0x500);
		SendDlgItemMessage(hCD_GFX, IDC_CD_GFX_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

		UpdateCD_GFX_KMod();
		UpdateWindow(hwnd);
		return 0;

	case WM_DRAWITEM:
		if ((UINT)wParam == IDC_CD_GFX)
			DrawCD_GFX_KMod((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_CD_GFX);
		break;

	case WM_DESTROY:
		cdgfx_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void cdgfx_create(HINSTANCE hInstance, HWND hWndParent)
{
	hCD_GFX = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGCD_GFX), hWndParent, CD_GFXDlgProc);
}

void cdgfx_show(BOOL visibility)
{
	ShowWindow(hCD_GFX, visibility ? SW_SHOW : SW_HIDE);
}

void cdgfx_update()
{
	if (OpenedWindow_KMod[DMODE_CD_GFX-1] == FALSE)	return;

	UpdateCD_GFX_KMod();
}

void cdgfx_reset()
{
	CD_GFXInit_KMod(hCD_GFX);
}
void cdgfx_destroy()
{
	DestroyWindow(hCD_GFX);
}

