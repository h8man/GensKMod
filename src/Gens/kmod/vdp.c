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
#include "vdp.h"
UCHAR pal_KMod;
static HWND hVDP;

static long palH, palV;
static UCHAR zoomTile_KMod, TileCurDraw; //, tileBank_KMod;
static USHORT tile_KMod;
static CHAR debug_string[1024];

static HWND hWndRedArrow;
static HWND hWndPal;
static RECT rcArrow, rcPal, rcTiles;

static const COLORREF cgaPal[16] = {
	RGB(0, 0, 0), RGB(0, 0, 0xAA), RGB(0, 0xAA, 0), RGB(0, 0xAA, 0xAA),
	RGB(0xAA, 0, 0), RGB(0xAA, 0, 0xAA), RGB(0xAA, 0x55, 0), RGB(0xAA, 0xAA, 0xAA),
	RGB(0x55, 0x55, 0x55), RGB(0x55, 0x55, 0xFF), RGB(0x55, 0xFF, 0x55), RGB(0x55, 0xFF, 0xFF),
	RGB(0xFF, 0x55, 0x55), RGB(0xFF, 0x55, 0xFF), RGB(0xFF, 0xFF, 0x55), RGB(0xFF, 0xFF, 0xFF),
};

static const COLORREF monoPal[16] = {
	RGB(0, 0, 0), RGB(0x11, 0x11, 0x11), RGB(0x22, 0x22, 0x22), RGB(0x33, 0x33, 0x33),
	RGB(0x44, 0x44, 0x44), RGB(0x55, 0x55, 0x55), RGB(0x66, 0x66, 0x66), RGB(0x77, 0x77, 0x77),
	RGB(0x88, 0x88, 0x88), RGB(0x99, 0x99, 0x99), RGB(0xAA, 0xAA, 0xAA), RGB(0xBB, 0xBB, 0xBB),
	RGB(0xCC, 0xCC, 0xCC), RGB(0xDD, 0xDD, 0xDD), RGB(0xEE, 0xEE, 0xEE), RGB(0xFF, 0xFF, 0xFF),
};

#define VDP_PALETTES 6
#define VDP_PALETTE_CGA 4
#define VDP_PALETTE_MONO 5

void UpdateVDP_KMod()
{
	RedrawWindow(hWndPal, NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(hVDP, IDC_VDP_TILES), NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(hVDP, IDC_VDP_PREVIEW), NULL, NULL, RDW_INVALIDATE);

	TileCurDraw++;
	if (TileCurDraw > 3)	TileCurDraw = 0;
}

void DrawVDP_Pal_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	unsigned char i, j;
	HBRUSH newBrush = NULL;
	RECT rc;

	for (j = 0; j < VDP_PALETTES; j++)
	{
		rc.top = j* palV;
		rc.bottom = rc.top + palV;

		for (i = 0; i < 16; i++)
		{
			if (newBrush)	DeleteObject((HGDIOBJ)newBrush);
			rc.left = i*palH;
			rc.right = rc.left + palH;

			newBrush = CreateSolidBrush(vdpdebug_getColor(j, i));

			FillRect(hlDIS->hDC, &rc, newBrush);
		}
	}

	if (newBrush)	DeleteObject((HGDIOBJ)newBrush);

	MoveWindow(hWndRedArrow, rcPal.left - (rcArrow.right - rcArrow.left), rcPal.top + pal_KMod*palV + (palV - (rcArrow.bottom - rcArrow.top)) / 2, (rcArrow.right - rcArrow.left), (rcArrow.bottom - rcArrow.top), TRUE);
}


void DrawVDP_Tiles_KMod(LPDRAWITEMSTRUCT hlDIS)
{

	WORD posX, posY, maxY, maxX;
	unsigned short int numTile = 0;
	SCROLLINFO si;

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, hlDIS->rcItem.right - hlDIS->rcItem.left, hlDIS->rcItem.bottom - hlDIS->rcItem.top);
	hOldBitmap = SelectObject(hDC, hBitmap);

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo((HWND)GetDlgItem(hVDP, IDC_VDP_SCROLL), SB_CTL, &si);

	numTile = si.nPos * 32; //tileBank_KMod*16;
	maxY = 32 * 8;
	maxX = 32 * 8;

	for (posY = 0; posY < maxY; posY += 8)
	{
		for (posX = 0; posX < maxX; posX += 8)
		{
			vdpdebug_drawTile(hDC, numTile++, posX, posY, pal_KMod, 1);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		(Paused ? 0 : TileCurDraw*maxY),  // y-coordinate of destination rectangle's upper-left
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

void DrawVDPZoom_Tiles_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	vdpdebug_drawTile(hlDIS->hDC, tile_KMod, 0, 0, pal_KMod, zoomTile_KMod);
}

void DumpTiles_KMod(HWND hwnd)
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

	COLORREF		tmpColor;
	DWORD			j;

	unsigned short int numTile;
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
	szFile.lpstrTitle = "Convert VRAM to bitmap";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bmp";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + 64 * 2000 / 2); // 4bits per pixel
	bmfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));


	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = 16 * 8;
	bmiHeader.biHeight = 125 * 8;
	bmiHeader.biPlanes = 0;
	bmiHeader.biBitCount = 4;
	bmiHeader.biClrUsed = 16;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 64 * 2000 / 2;
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


	numTile = 0;
	for (posY = 0; posY < 125; posY++)
	{
		for (posX = 0; posX < 16; posX++)
		{
			for (j = 0; j < 8; j++)
			{
				TileData = VRam[numTile * 32 + j * 4 + 1];
				pBits[posX * 4 + (124 - posY) * 8 * 16 * 4 + (7 - j) * 16 * 4] = TileData;

				TileData = VRam[numTile * 32 + j * 4];
				pBits[posX * 4 + (124 - posY) * 8 * 16 * 4 + (7 - j) * 16 * 4 + 1] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 3];
				pBits[posX * 4 + (124 - posY) * 8 * 16 * 4 + (7 - j) * 16 * 4 + 2] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 2];
				pBits[posX * 4 + (124 - posY) * 8 * 16 * 4 + (7 - j) * 16 * 4 + 3] = TileData;
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

	Put_Info("Tiles dumped", 1500);
}

void DumpVRAM_KMod(HWND hwnd)
{
	OPENFILENAME szFile;
	char szFileName[MAX_PATH];
	HANDLE hFr;
	DWORD dwBytesToWrite, dwBytesWritten;

	ZeroMemory(&szFile, sizeof(szFile));
	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */

	strcpy(szFileName, Rom_Name);
	strcat(szFileName, "_VRAM");

	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump VRAM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 64 * 1024;
	WriteFile(hFr, VRam, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("VRAM dumped", 1500);
}


void DumpCRAM_KMod(HWND hwnd)
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
	szFile.lpstrFilter = "RAM dump (*.ram)\0*.ram\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump CRAM";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "ram";

	if (GetSaveFileName(&szFile) != TRUE)   return;

	hFr = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFr == INVALID_HANDLE_VALUE)
		return;

	dwBytesToWrite = 64 * 8;
	WriteFile(hFr, CRam, dwBytesToWrite, &dwBytesWritten, NULL);

	CloseHandle(hFr);

	Put_Info("CRAM dumped", 1500);
}

void VDPPal_Choose()
{
	POINT pt;

	GetCursorPos(&pt);
	ScreenToClient(hVDP, &pt);
	if (!PtInRect(&rcPal, pt))        return;

	pt.y -= rcPal.top;
	pal_KMod = (unsigned char)(pt.y / palV);

	if (Paused)	UpdateVDP_KMod();
}

void VDPTile_Choose()
{
	POINT pt;
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo((HWND)GetDlgItem(hVDP, IDC_VDP_SCROLL), SB_CTL, &si);

	GetCursorPos(&pt);
	ScreenToClient(hVDP, &pt);

	if (!PtInRect(&rcTiles, pt))        return;

	pt.y -= rcTiles.top;
	tile_KMod = (unsigned char)(pt.y / 8);
	tile_KMod *= 32;
	tile_KMod += si.nPos * 32;


	pt.x -= rcTiles.left;
	tile_KMod += (unsigned char)(pt.x / 8);

	wsprintf(debug_string, "Tile 0x%.4X", tile_KMod);
	SendDlgItemMessage(hVDP, IDC_VDP_TILE_ZOOM_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

	if (Paused)	UpdateVDP_KMod();
}


void VDPInit_KMod(HWND hwnd)
{
	RECT TZoomRect;
	SCROLLINFO si;

	GetClientRect(GetDlgItem(hwnd, IDC_VDP_PREVIEW), &TZoomRect);

	zoomTile_KMod = (UCHAR)((TZoomRect.right - TZoomRect.left) / 8);
	zoomTile_KMod = (UCHAR)min(zoomTile_KMod, (TZoomRect.bottom - TZoomRect.top) / 8);

	pal_KMod = 0;
	//	tileBank_KMod = 0;

	hWndPal = GetDlgItem(hwnd, IDC_VDP_PAL);
	GetClientRect(hWndPal, &rcPal);
	palV = (rcPal.bottom - rcPal.top) / VDP_PALETTES;//4;
	palH = (rcPal.right - rcPal.left) / 16;

	hWndRedArrow = GetDlgItem(hwnd, IDC_VDP_PAL_ARROW);
	GetClientRect(hWndRedArrow, &rcArrow);

	MapWindowPoints(hWndPal, hwnd, (LPPOINT)&rcPal, 2);

	GetClientRect(GetDlgItem(hwnd, IDC_VDP_TILES), &rcTiles);
	MapWindowPoints(GetDlgItem(hwnd, IDC_VDP_TILES), hwnd, (LPPOINT)&rcTiles, 2);

	rcTiles.bottom = rcTiles.top + 32 * 8;
	rcTiles.right = rcTiles.left + 32 * 8;
	MoveWindow(GetDlgItem(hwnd, IDC_VDP_TILES), rcTiles.left, rcTiles.top, rcTiles.right - rcTiles.left, rcTiles.bottom - rcTiles.top, TRUE);
	MoveWindow(GetDlgItem(hwnd, IDC_VDP_SCROLL), rcTiles.right, rcTiles.top, GetSystemMetrics(SM_CXVSCROLL), rcTiles.bottom - rcTiles.top, TRUE);

	MoveWindow(GetDlgItem(hwnd, IDC_VDP_TILE_ZOOM_ADR), rcTiles.left, rcTiles.bottom + 8, TZoomRect.right - TZoomRect.left, 16, TRUE);
	MoveWindow(GetDlgItem(hwnd, IDC_VDP_PREVIEW), rcTiles.left, rcTiles.bottom + 8 + 16 + 4, TZoomRect.right - TZoomRect.left, TZoomRect.bottom - TZoomRect.top, TRUE);

	CheckDlgButton(hwnd, IDC_VDP_PAL_1, BST_CHECKED);
	CheckDlgButton(hwnd, IDC_VDP_PAL_2, BST_CHECKED);
	CheckDlgButton(hwnd, IDC_VDP_PAL_3, BST_CHECKED);
	CheckDlgButton(hwnd, IDC_VDP_PAL_4, BST_CHECKED);
	ActivePal = 0x0F;

	TileCurDraw = 0;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nPage = 8;
	si.nPos = 0;
	si.nMax = 32;

	SetScrollInfo(GetDlgItem(hwnd, IDC_VDP_SCROLL), SB_CTL, &si, TRUE);
}


BOOL CALLBACK VDPDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO si;

	switch (Message)
	{
	case WM_INITDIALOG:
		VDPInit_KMod(hwnd);
		break;

	case WM_DRAWITEM:
		if ((UINT)wParam == IDC_VDP_PAL)
			DrawVDP_Pal_KMod((LPDRAWITEMSTRUCT)lParam);
		else if ((UINT)wParam == IDC_VDP_TILES)
			DrawVDP_Tiles_KMod((LPDRAWITEMSTRUCT)lParam);
		else if ((UINT)wParam == IDC_VDP_PREVIEW)
			DrawVDPZoom_Tiles_KMod((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_VDP_PAL:
			VDPPal_Choose();
			break;
		case IDC_VDP_TILES:
			VDPTile_Choose();
			break;
		case IDC_VDP_VRAM:
			DumpVRAM_KMod(hwnd);
			break;
		case IDC_VDP_CRAM:
			DumpCRAM_KMod(hwnd);
			break;
			/*
			case IDC_VDP_TILES_UP:
			if (tileBank_KMod)	tileBank_KMod--;
			wsprintf(debug_string, "0x%.4X", tileBank_KMod*16);
			SendDlgItemMessage(hVDP , IDC_VDP_TILES_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

			if (Paused)	UpdateVDP_KMod( );
			break;
			case IDC_VDP_TILES_DWN:
			if (tileBank_KMod < 96) tileBank_KMod++;
			wsprintf(debug_string, "0x%.4X", tileBank_KMod*16);
			SendDlgItemMessage(hVDP , IDC_VDP_TILES_ADR, WM_SETTEXT, 0 , (LPARAM)debug_string);

			if (Paused)	UpdateVDP_KMod( );
			break;
			*/
		case IDC_VDP_BMP:
			DumpTiles_KMod(hwnd);
			break;
		case IDC_VDP_PAL_1:
			if (IsDlgButtonChecked(hwnd, IDC_VDP_PAL_1) == BST_CHECKED)
				ActivePal |= 0x01;
			else
				ActivePal &= 0x0E;
			CRam_Flag = 1;
			break;
		case IDC_VDP_PAL_2:
			if (IsDlgButtonChecked(hwnd, IDC_VDP_PAL_2) == BST_CHECKED)
				ActivePal |= 0x02;
			else
				ActivePal &= 0x0D;
			CRam_Flag = 1;
			break;
		case IDC_VDP_PAL_3:
			if (IsDlgButtonChecked(hwnd, IDC_VDP_PAL_3) == BST_CHECKED)
				ActivePal |= 0x04;
			else
				ActivePal &= 0x0B;
			CRam_Flag = 1;
			break;
		case IDC_VDP_PAL_4:
			if (IsDlgButtonChecked(hwnd, IDC_VDP_PAL_4) == BST_CHECKED)
				ActivePal |= 0x08;
			else
				ActivePal &= 0x07;
			CRam_Flag = 1;
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

		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		SetScrollInfo((HWND)lParam, SB_CTL, &si, TRUE);

		wsprintf(debug_string, "Offset 0x%.4X", si.nPos * 32);
		SendDlgItemMessage(hVDP, IDC_VDP_TILES_ADR, WM_SETTEXT, 0, (LPARAM)debug_string);

		UpdateVDP_KMod();
		UpdateWindow(hwnd);
		return 0;

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_VDP);
		break;

	case WM_DESTROY:
		vdpdebug_destroy();
		PostQuitMessage(0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void vdpdebug_create(HINSTANCE hInstance, HWND hWndParent)
{
	hVDP = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGVDP), hWndParent, VDPDlgProc);
}

void vdpdebug_show(BOOL visibility)
{
	ShowWindow(hVDP, visibility ? SW_SHOW : SW_HIDE);
}

void vdpdebug_update()
{
	if (OpenedWindow_KMod[DMODE_VDP-1] == FALSE)	return;

	UpdateVDP_KMod();
}

void vdpdebug_reset()
{

}
void vdpdebug_destroy()
{
	DestroyWindow(hVDP);
}


BOOL vdpdebug_isMessage(LPMSG lpMsg)
{
		if (IsDialogMessage(hVDP, lpMsg))
		{
			TranslateAccelerator(hVDP, hAccelTable, lpMsg);
			return TRUE;
		}
	

	return FALSE;
}


void vdpdebug_drawTile(HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom)
{
	int j;
	COLORREF tile_data[8 * 8];
	COLORREF *ptr = &tile_data[0];
	unsigned int tile_row;

	static const struct BITMAPINFO_LOCAL
	{
		BITMAPINFOHEADER hdr;
		COLORREF colors[3];
	}
	bmi =
	{
		{
			sizeof(BITMAPINFOHEADER),   // biSize
			8,                          // biWidth
			-8,                         // biHeight
			1,                          // biPlanes
			32,                         // biBitCount
			BI_BITFIELDS,               // biCompression
			0,                          // biSizeImage
			75,                         // biXPelsPerMeter
			75,                         // biYPelsPerMeter
			0,                          // biClrUsed
			0                           // biClrImportant
		},
		{
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
		}
	};
	const unsigned int * __restrict vram_dwords = (const unsigned int *)VRam;

	for (j = 0; j < 8; j++)
	{
		tile_row = vram_dwords[numTile * 8 + j];
		ptr[3] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[2] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[1] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[0] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[7] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[6] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[5] = vdpdebug_getColor(pal, tile_row & 0xF);
		tile_row >>= 4;
		ptr[4] = vdpdebug_getColor(pal, tile_row & 0xF);
		ptr += 8;
	}

	StretchDIBits(
		hDCMain,
		x, y,
		8 * zoom, 8 * zoom,
		0, 0,
		8, 8,
		tile_data,
		(const BITMAPINFO *)&bmi,
		DIB_RGB_COLORS,
		SRCCOPY);
}

COLORREF vdpdebug_getColor(unsigned int numPal, unsigned int numColor)
{
	/* can't use MD_Palette since it's DirectX mode (555 or 565) */
	/* !! CRAM is (binary:)GGG0RRR00000BBB0   while COLORREF is (hexa:)0x00BBGGRR */
	unsigned short int md_color;
	COLORREF newColor;

	// handle our false CGA pal
	if (numPal == VDP_PALETTE_CGA)	return (cgaPal[numColor]);
	if (numPal == VDP_PALETTE_MONO)	return (monoPal[numColor]);

	/* B */
	newColor = (COLORREF)CRam[2 * 16 * numPal + 2 * numColor + 1];
	newColor <<= 20;

	/* G */
	md_color = (unsigned short int) (CRam[2 * 16 * numPal + 2 * numColor] & 0xF0);
	md_color <<= 8;
	newColor |= md_color;

	/* R */
	md_color = (unsigned short int) (CRam[2 * 16 * numPal + 2 * numColor] & 0x0F);
	md_color <<= 4;
	newColor |= md_color;

	//transform genny white to true white
	if (newColor == 0x00E0E0E0)
		newColor = 0x00FFFFFF;

	return newColor;
}