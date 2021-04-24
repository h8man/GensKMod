#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_io.h"
#include "../G_gfx.h" //used for Put_Info


//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "utils.h"
#include "sprites.h"
#include "vdp.h"


static HWND hSprites;
static HWND hSpriteList;

unsigned char TrueSize_KMod(unsigned short int data)
{
	switch (data)
	{
	case 0:
		return 8;
	case 1:
		return 16;
	case 2:
		return 24;
	case 3:
		return 32;
	}

	return 0;
}


void SpritesInit_KMod(HWND hDlg)
{
	LV_COLUMN   lvColumn;
	int         i;
	TCHAR       szString[8][7] = { "Num", "Ypos", "XPos", "Size", "Link", "Pal", "Tile", "Flags*" };
	LVITEM		lvItem;
	char		buf[64];


	hSpriteList = GetDlgItem(hDlg, IDC_SPRITES_LIST);
	ListView_DeleteAllItems(hSpriteList);

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 53;
	for (i = 0; i < 8; i++)
	{
		lvColumn.pszText = szString[i];
		ListView_InsertColumn(hSpriteList, i, &lvColumn);
	}

	ListView_SetExtendedListViewStyle(hSpriteList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	for (i = 0; i < 80; i++)
	{
		wsprintf(buf, "%0.2d", i);

		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = buf;
		ListView_InsertItem(hSpriteList, &lvItem);
	}

}

void UpdateSprites_KMod()
{
	unsigned int i;
	unsigned short int *sprData, tmp;
	unsigned char tmp_string[32];
	static unsigned short data_copy[80 * 4];
	static unsigned int forceRefreshCounter;
	LVITEM		lvItem;

	sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));


	if (!memcmp(sprData, data_copy, sizeof(data_copy)))
	{
		//sometimes miss the last update... so force refresh every n frame
		if (--forceRefreshCounter > 0)	return;
	}
	forceRefreshCounter = 0xFFFF;
	memcpy(data_copy, sprData, sizeof(data_copy));


	for (i = 0; i < 80; i++)
	{
		lvItem.iItem = i;
		lvItem.iSubItem = LVIF_TEXT;
		ListView_GetItem(hSpriteList, &lvItem);

		/*
		// not needed
		// index
		wsprintf(tmp_string, "%.2d", i);
		ListView_SetItemText(hSpriteList, i, 0, tmp_string);
		*/

		// yPos
		wsprintf(tmp_string, "%4d", sprData[0] & 0x03FF);
		ListView_SetItemText(hSpriteList, i, 1, tmp_string);

		// xPos
		wsprintf(tmp_string, "%4d", sprData[3] & 0x03FF);
		ListView_SetItemText(hSpriteList, i, 2, tmp_string);

		// size
		wsprintf(tmp_string, "%.2dx%.2d",
			TrueSize_KMod(((sprData[1] & 0x0C00) >> 10)),
			TrueSize_KMod(((sprData[1] & 0x0300) >> 8)));
		ListView_SetItemText(hSpriteList, i, 3, tmp_string);

		// link
		wsprintf(tmp_string, "%.2d", sprData[1] & 0x007F);
		ListView_SetItemText(hSpriteList, i, 4, tmp_string);

		// pal
		wsprintf(tmp_string, "%2d", (sprData[2] & 0x6000) >> 13);
		ListView_SetItemText(hSpriteList, i, 5, tmp_string);

		// tile
		wsprintf(tmp_string, "%4d", (sprData[2] & 0x07FF));
		ListView_SetItemText(hSpriteList, i, 6, tmp_string);

		// flags
		tmp = 0;
		/*
		if ( sprData[2]&0x8000)	tmp +=100;
		if ( sprData[2]&0x1000)	tmp +=010;
		if ( sprData[2]&0x0800)	tmp +=001;
		wsprintf(tmp_string, "%.3d", tmp);
		*/
		if (sprData[2] & 0x8000)	tmp |= 4;
		if (sprData[2] & 0x1000)	tmp |= 2;
		if (sprData[2] & 0x0800)	tmp |= 1;
		wsprintf(tmp_string, "%c%c%c", tmp & 4 ? 'P' : '0', tmp & 2 ? 'V' : '0', tmp & 1 ? 'H' : '0');
		ListView_SetItemText(hSpriteList, i, 7, tmp_string);

		sprData += 4;
	}

	RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
}


void DrawSprite_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	unsigned int selIdx;
	unsigned short int *sprData, pal;
	WORD posX, posY;
	unsigned char sizeH, sizeV;
	unsigned short int numTile = 0;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;

	sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
	sprData += selIdx * 4; /* each sprite is 4 short int data */

	numTile = sprData[2] & 0x07FF;
	sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
	sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, 32, 32);
	hOldBitmap = SelectObject(hDC, hBitmap);
	FillRect(hDC, &(hlDIS->rcItem), (HBRUSH)(COLOR_3DFACE + 1));

	pal = (sprData[2] & 0x6000) >> 13;
	for (posX = 0; posX < sizeH; posX += 8)
	{
		for (posY = 0; posY < sizeV; posY += 8)
		{
			vdpdebug_drawTile(hDC, numTile++, posX, posY, (UCHAR)pal, 1);
		}
	}

	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		0,  // y-coordinate of destination rectangle's upper-left
		// corner
		32,  // width of destination rectangle
		32, // height of destination rectangle
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


void DrawSpriteZoom_KMod(LPDRAWITEMSTRUCT hlDIS)
{
	unsigned int selIdx;
	unsigned short int *sprData, pal;
	WORD posX, posY;
	unsigned char sizeH, sizeV, zoom;
	unsigned short int numTile = 0;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;

	sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
	sprData += selIdx * 4; /* each sprite is 4 short int data */

	numTile = sprData[2] & 0x07FF;
	sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
	sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));

	zoom = (unsigned char)((hlDIS->rcItem.right - hlDIS->rcItem.left) / 32);
	zoom = (unsigned char)min(zoom, (hlDIS->rcItem.bottom - hlDIS->rcItem.top) / 32);

	hDC = CreateCompatibleDC(hlDIS->hDC);
	hBitmap = CreateCompatibleBitmap(hlDIS->hDC, 32 * zoom, 32 * zoom);
	hOldBitmap = SelectObject(hDC, hBitmap);
	FillRect(hDC, &(hlDIS->rcItem), (HBRUSH)(COLOR_3DFACE + 1));

	pal = (sprData[2] & 0x6000) >> 13;
	for (posX = 0; posX < sizeH; posX += 8)
	{
		for (posY = 0; posY < sizeV; posY += 8)
		{
			vdpdebug_drawTile(hDC, numTile++, posX*zoom, posY*zoom, (UCHAR)pal, zoom);
		}
	}


	BitBlt(
		hlDIS->hDC, // handle to destination device context
		0,  // x-coordinate of destination rectangle's upper-left
		// corner
		0,  // y-coordinate of destination rectangle's upper-left
		// corner
		32 * zoom,  // width of destination rectangle
		32 * zoom, // height of destination rectangle
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

void DumpSprite_KMod(HWND hwnd)
{
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits;

	OPENFILENAME		szFile;
	CHAR				szFileName[MAX_PATH];
	HANDLE				hFr;
	DWORD				dwBytesToWrite, dwBytesWritten;

	COLORREF			tmpColor;
	unsigned short int	numTile;
	int					selIdx, tmp;
	unsigned char		TileData, sizeH, sizeV, posX, posY, j, pal;
	unsigned short int	*sprData;

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;


	szFileName[0] = 0;  /*WITHOUT THIS, CRASH */
	wsprintf(szFileName, "%s_sprite_%.2d", Rom_Name, selIdx);

	ZeroMemory(&szFile, sizeof(szFile));
	szFile.lStructSize = sizeof(szFile);
	szFile.hwndOwner = hwnd;
	szFile.lpstrFilter = "16Colors Bitmap file (*.bmp)\0*.bmp\0\0";
	szFile.lpstrFile = szFileName;
	szFile.nMaxFile = sizeof(szFileName);
	szFile.lpstrFileTitle = (LPSTR)NULL;
	szFile.lpstrInitialDir = (LPSTR)NULL;
	szFile.lpstrTitle = "Dump sprite";
	szFile.Flags = OFN_EXPLORER | OFN_LONGNAMES | OFN_NONETWORKBUTTON |
		OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	szFile.lpstrDefExt = "bmp";

	if (GetSaveFileName(&szFile) != TRUE)   return;


	sprData = (unsigned short *)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
	sprData += selIdx * 4; /* each sprite is 4 short int data */

	numTile = sprData[2] & 0x07FF;
	sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
	sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));
	pal = (sprData[2] & 0x6000) >> 13;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = sizeH;
	bmiHeader.biHeight = sizeV;
	bmiHeader.biPlanes = 0;
	bmiHeader.biBitCount = 4;
	bmiHeader.biClrUsed = 16;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = sizeH*sizeV / 2;
	bmiHeader.biClrImportant = 0;


	bmfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bmfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + bmiHeader.biSizeImage);
	bmfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));


	for (j = 0; j < 16; j++)
	{
		tmpColor = vdpdebug_getColor(pal, j);
		bmiColors[j].rgbRed = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbGreen = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbBlue = (BYTE)tmpColor & 0xFF;
	}

	if (IsDlgButtonChecked(hSprites, IDC_CHECK_MAGENTAKEY))
	{
		bmiColors[0].rgbBlue = 0xff;
		bmiColors[0].rgbGreen = 0x00;
		bmiColors[0].rgbRed = 0xff;
	}

	pBits = (LPBYTE)LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}

	for (posX = 0; posX < sizeH; posX += 8)
	{
		for (posY = 0; posY < sizeV; posY += 8)
		{
			for (j = 0; j < 8; j++)
			{
				TileData = VRam[numTile * 32 + j * 4 + 1];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j))*sizeH / 2;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j))*sizeH / 2 + 1;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 3];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j))*sizeH / 2 + 2;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 2];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j))*sizeH / 2 + 3;
				pBits[tmp] = TileData;
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

	Put_Info("Sprite dumped", 1500);

}

void CopySprite_KMod(HWND hwnd)
{
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD				bmiColors[16];
	LPBYTE				pBits, pBmp;

	DWORD				dwBytesToWrite, dwBytesWritten;

	COLORREF			tmpColor;
	unsigned short int	numTile;
	int					selIdx, tmp;
	unsigned char		TileData, sizeH, sizeV, posX, posY, j, pal;
	unsigned short int* sprData;
	size_t				bmpSize;

	selIdx = SendMessage(hSpriteList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED); // return item selected
	if (selIdx == -1)
		return;

	sprData = (unsigned short*)(VRam + (VDP_Reg.Spr_Att_Adr << 9));
	sprData += selIdx * 4; /* each sprite is 4 short int data */

	numTile = sprData[2] & 0x07FF;
	sizeH = TrueSize_KMod(((sprData[1] & 0x0C00) >> 10));
	sizeV = TrueSize_KMod(((sprData[1] & 0x0300) >> 8));
	pal = (sprData[2] & 0x6000) >> 13;

	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = sizeH;
	bmiHeader.biHeight = sizeV;
	bmiHeader.biPlanes = 0;
	bmiHeader.biBitCount = 4;
	bmiHeader.biClrUsed = 16;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = sizeH * sizeV / 2;
	bmiHeader.biClrImportant = 0;

	for (j = 0; j < 16; j++)
	{
		tmpColor = vdpdebug_getColor(pal, j);;
		bmiColors[j].rgbRed = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbGreen = (BYTE)tmpColor & 0xFF;
		tmpColor >>= 8;
		bmiColors[j].rgbBlue = (BYTE)tmpColor & 0xFF;
	}

	if (IsDlgButtonChecked(hSprites, IDC_CHECK_MAGENTAKEY))
	{
		bmiColors[0].rgbBlue = 0xff;
		bmiColors[0].rgbGreen = 0x00;
		bmiColors[0].rgbRed = 0xff;
	}

	pBits = (LPBYTE)LocalAlloc(LPTR, bmiHeader.biSizeImage);
	if (pBits == NULL)
	{
		return;
	}

	for (posX = 0; posX < sizeH; posX += 8)
	{
		for (posY = 0; posY < sizeV; posY += 8)
		{
			for (j = 0; j < 8; j++)
			{
				TileData = VRam[numTile * 32 + j * 4 + 1];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j)) * sizeH / 2;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j)) * sizeH / 2 + 1;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 3];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j)) * sizeH / 2 + 2;
				pBits[tmp] = TileData;

				TileData = VRam[numTile * 32 + j * 4 + 2];
				tmp = posX / 2 + ((sizeV - 1) - (posY + j)) * sizeH / 2 + 3;
				pBits[tmp] = TileData;
			}
			numTile++;
		}
	}

	bmpSize = sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD) + bmiHeader.biSizeImage;
	pBmp = (LPBYTE)LocalAlloc(LPTR, bmpSize);
	if (pBmp == NULL)
	{
		LocalFree((HLOCAL)pBits);
		return;
	}


	dwBytesWritten = 0;
	dwBytesToWrite = sizeof(BITMAPINFOHEADER);
	memcpy(pBmp + dwBytesWritten, &bmiHeader, dwBytesToWrite);

	dwBytesWritten += dwBytesToWrite;
	dwBytesToWrite = 16 * sizeof(RGBQUAD);
	memcpy(pBmp + dwBytesWritten, bmiColors, dwBytesToWrite);

	dwBytesWritten += dwBytesToWrite;
	dwBytesToWrite = bmiHeader.biSizeImage;
	memcpy(pBmp + dwBytesWritten, pBits, dwBytesToWrite);

	CopyToClipboard(CF_DIB, pBmp, bmpSize, 1);

	LocalFree((HLOCAL)pBits);
	LocalFree((HLOCAL)pBmp);

	Put_Info("Sprite copied", 1500);

}


BOOL CALLBACK SpritesDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		sprites_reset();

		break;
	case WM_DRAWITEM:
		if ((UINT)wParam == IDC_SPRITES_PREVIEW)
			DrawSprite_KMod((LPDRAWITEMSTRUCT)lParam);
		else if ((UINT)wParam == IDC_SPRITES_PREVIEW2)
			DrawSpriteZoom_KMod((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SPRITES_DUMP:
			DumpSprite_KMod(hSprites);
			break;
		case IDC_SPRITES_COPY:
			CopySprite_KMod(hSprites);
			break;
		case IDC_SPRITES_FRAMESTEP:
			FrameStep_KMod();
			break;
		}

		break;

	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case IDC_SPRITES_LIST:
			if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED)
			{
				RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW), NULL, NULL, RDW_INVALIDATE);
				RedrawWindow(GetDlgItem(hSprites, IDC_SPRITES_PREVIEW2), NULL, NULL, RDW_INVALIDATE);
			}
			break;
		}
		break;
	case WM_CLOSE:
		CloseWindow_KMod(DMODE_SPRITES);
		break;

	case WM_DESTROY:
		sprites_destroy();
		
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


void sprites_create(HINSTANCE hInstance, HWND hWndParent)
{
	hSprites = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGSPRITES), hWndParent, SpritesDlgProc);
}

void sprites_show(BOOL visibility)
{
	ShowWindow(hSprites, visibility ? SW_SHOW : SW_HIDE);
}

void sprites_update()
{
	if (OpenedWindow_KMod[DMODE_VDP-1] == FALSE)	return;

	UpdateSprites_KMod();
}

void sprites_reset()
{
	SpritesInit_KMod(hSprites);
}
void sprites_destroy()
{
	ListView_DeleteAllItems(hSpriteList);
	DestroyWindow(hSprites);
}
