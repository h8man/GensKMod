#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include "../gens.h"
#include "../resource.h"
#include "../vdp_io.h"
#include "../vdp_rend.h"
#include "../scrshot.h"
//TODO remove to use right header(s)
//#include "../kmod.h"

#include "common.h"
#include "vdp_composite.h"

//#define VDP_COMPOSITE_DIRECT_BLIT 1

HWND hPlanesComposite;
static BOOL show_transparence = FALSE;

static unsigned char debug_base[][3] = {
	{127, 127, 127}, //BG
	{0, 127, 0},     //B
	{0, 0, 127},     //A
	{127, 0, 0},     //Window
	{127, 0, 127}    //Sprites
};

static void PlaneCompositeInit_KMod(HWND hDlg)
{
	HWND hcomposite;
	RECT rc;

	//TODO add support for 32X ?

	InitCommonControls();

	hcomposite = (HWND)GetDlgItem(hDlg, IDC_PLANECOMPOSITE_MAIN);
	GetClientRect(hDlg, &rc);
	MoveWindow(hcomposite, 20, 60, (rc.right - rc.left) - 40, (rc.bottom - rc.top) - 80, TRUE);
}

DWORD ColorAproximation(unsigned short color, int mode)
{
	unsigned int r, g, b;
	if (mode)
	{

		r = ((color >> 7) & 0xF8);
		g = ((color >> 2) & 0xF8);
		b = ((color << 3) & 0xF8);
	}
	else
	{
		
		r = ((color >> 8) & 0xF8);
		g = ((color >> 3) & 0xFC);
		b = ((color << 3) & 0xF8);

	}
	return RGB(r, g, b);
}

static HBITMAP CreateScreenBitmap(HDC hdc, const unsigned short *screen, int height, int stride)
{
	struct
	{
		BITMAPINFOHEADER header;
		DWORD masks[3];
	} bitmapInfo;
	HBITMAP bitmap;
	unsigned short *bits;

	memset(&bitmapInfo, 0, sizeof(bitmapInfo));
	bitmapInfo.header.biSize = sizeof(bitmapInfo.header);
	bitmapInfo.header.biWidth = stride / sizeof(*bits);
	bitmapInfo.header.biHeight = -height;
	bitmapInfo.header.biPlanes = 1;
	bitmapInfo.header.biBitCount = 16;
	bitmapInfo.header.biCompression = BI_BITFIELDS;
	bitmapInfo.masks[0] = Mode_555 & 1 ? 0x7C00 : 0xF800;
	bitmapInfo.masks[1] = Mode_555 & 1 ? 0x03E0 : 0x07E0;
	bitmapInfo.masks[2] = 0x001F;

	bitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS, (void **)&bits, NULL, 0);
	if (bitmap != NULL)
		memcpy(bits, screen, stride * height);

	return bitmap;
}

static void DrawCompositeBackground(HDC hdc, const RECT *rect)
{
	HBRUSH background;
	HBRUSH checkerBackground;
	RECT checkerRect;
	int x, y;

	if (!show_transparence)
	{
		background = CreateSolidBrush(RGB(debug_base[0][0], debug_base[0][1], debug_base[0][2]));
		FillRect(hdc, rect, background);
		DeleteObject(background);
		return;
	}

	background = CreateSolidBrush(RGB(44, 44, 44));
	checkerBackground = CreateSolidBrush(RGB(55, 55, 55));
	for (y = rect->top; y < rect->bottom; y += 8)
	{
		for (x = rect->left; x < rect->right; x += 8)
		{
			checkerRect.left = x;
			checkerRect.top = y;
			checkerRect.right = (x + 8 < rect->right) ? x + 8 : rect->right;
			checkerRect.bottom = (y + 8 < rect->bottom) ? y + 8 : rect->bottom;
			FillRect(hdc, &checkerRect, ((x / 8) + (y / 8)) & 1 ? background : checkerBackground);
		}
	}
	DeleteObject(checkerBackground);
	DeleteObject(background);
}

static BOOL CreateCompositeBuffer(HDC targetDC, int width, int height, HDC *bufferDC, HBITMAP *bufferBitmap, HBITMAP *oldBitmap)
{
	*bufferDC = CreateCompatibleDC(targetDC);
	*bufferBitmap = CreateCompatibleBitmap(targetDC, width, height);
	if (*bufferDC == NULL || *bufferBitmap == NULL)
	{
		if (*bufferBitmap != NULL)
			DeleteObject(*bufferBitmap);
		if (*bufferDC != NULL)
			DeleteDC(*bufferDC);
		return FALSE;
	}

	*oldBitmap = (HBITMAP)SelectObject(*bufferDC, *bufferBitmap);
	return TRUE;
}

static void DestroyCompositeBuffer(HDC bufferDC, HBITMAP bufferBitmap, HBITMAP oldBitmap)
{
	SelectObject(bufferDC, oldBitmap);
	DeleteObject(bufferBitmap);
	DeleteDC(bufferDC);
}

static void BlitTransform(HDC hdcDst, HBITMAP hBmpSource, int width, int height, int sourceX, int posX, int posY, COLORREF color)
{
	COLORREF bkColor;
	HDC hdcMask = NULL;
	HDC hdcInverseMask = NULL;
	HDC hdcSource = NULL;
	HDC hdcColor = NULL;
	HBITMAP hBmpMask = NULL;
	HBITMAP hBmpInverseMask = NULL;
	HBITMAP hBmpColor = NULL;
	HBITMAP hOldMask;
	HBITMAP hOldInverseMask;
	HBITMAP hOldSource;
	HBITMAP hOldColor;
	HBRUSH brush;
	RECT rect;

	// 1. Create monochrome masks from the source's black background.
	hdcMask = CreateCompatibleDC(hdcDst);
	hdcInverseMask = CreateCompatibleDC(hdcDst);
	hdcSource = CreateCompatibleDC(hdcDst);
	hdcColor = CreateCompatibleDC(hdcDst);
	hBmpMask = CreateBitmap(width, height, 1, 1, NULL);
	hBmpInverseMask = CreateBitmap(width, height, 1, 1, NULL);
	hBmpColor = CreateCompatibleBitmap(hdcDst, width, height);
	if (hdcMask == NULL || hdcInverseMask == NULL || hdcSource == NULL || hdcColor == NULL ||
		hBmpMask == NULL || hBmpInverseMask == NULL || hBmpColor == NULL)
		goto cleanup;

	hOldMask = (HBITMAP)SelectObject(hdcMask, hBmpMask);
	hOldInverseMask = (HBITMAP)SelectObject(hdcInverseMask, hBmpInverseMask);
	hOldSource = (HBITMAP)SelectObject(hdcSource, hBmpSource);
	hOldColor = (HBITMAP)SelectObject(hdcColor, hBmpColor);

	#ifdef VDP_COMPOSITE_DIRECT_BLIT
	BitBlt(hdcDst, posX, posY, width, height, hdcSource, sourceX, 0, SRCCOPY);
	goto restore;
	#endif

	// Matching source pixels become white (transparent); all others become black.
	bkColor = ColorAproximation(MD_Palette[0], Mode_555 & 1);

	SetBkColor(hdcSource, bkColor);
	BitBlt(hdcMask, 0, 0, width, height, hdcSource, sourceX, 0, SRCCOPY);
	BitBlt(hdcInverseMask, 0, 0, width, height, hdcMask, 0, 0, NOTSRCCOPY);

	//return 	BitBlt(hdcDst, posX, posY, width, height, hdcInverseMask, 0, 0, SRCPAINT);
	// 2. Create a solid layer-color bitmap.
	brush = CreateSolidBrush(color);
	rect.left = rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	FillRect(hdcColor, &rect, brush);
	DeleteObject(brush);

	// 3. Keep the layer color only where the source bitmap is not black.
	BitBlt(hdcColor, 0, 0, width, height, hdcInverseMask, 0, 0, SRCAND);

	// 4. Preserve the destination at transparent pixels and composite the layer color.
	BitBlt(hdcDst, posX, posY, width, height, hdcMask, 0, 0, SRCAND);
	BitBlt(hdcDst, posX, posY, width, height, hdcColor, 0, 0, SRCPAINT);

	#ifdef VDP_COMPOSITE_DIRECT_BLIT
restore:
	#endif
	// Restore selected objects before releasing the temporary GDI resources.
	SelectObject(hdcColor, hOldColor);
	SelectObject(hdcSource, hOldSource);
	SelectObject(hdcInverseMask, hOldInverseMask);
	SelectObject(hdcMask, hOldMask);

cleanup:
	// Release all temporary bitmaps and device contexts.
	if (hBmpColor != NULL)
		DeleteObject(hBmpColor);
	if (hBmpInverseMask != NULL)
		DeleteObject(hBmpInverseMask);
	if (hBmpMask != NULL)
		DeleteObject(hBmpMask);
	if (hdcColor != NULL)
		DeleteDC(hdcColor);
	if (hdcSource != NULL)
		DeleteDC(hdcSource);
	if (hdcInverseMask != NULL)
		DeleteDC(hdcInverseMask);
	if (hdcMask != NULL)
		DeleteDC(hdcMask);
}

static void RenderCompositeLayers(HDC hdc, int width, int height, int stride, int sidebar)
{
	//case IDC_LAYER_B: ActiveLayer = 0x04;
	//case IDC_LAYER_A:	ActiveLayer = 0x08;
	//case IDC_LAYER_WINDOW: ActiveLayer = 0x01;
	//case IDC_LAYER_SPRITE: ActiveLayer = 0x02;

	//case IDC_LAYER_32X: ActiveLayer = 0x10;
	static const UCHAR layers[] = { 0x04, 0x08 | 0x01, 0x01, 0x02 };
	static const int colors[] = { 1,2,3,4 };
	static const int priority[] = { 0x01, 0x10 };
	UCHAR oldActiveLayer = ActiveLayer;
	int p;
	int i;

	for (p = 0; p < sizeof(priority) / sizeof(priority[0]); p++)
	{
		HBITMAP screenBitmap;

		PriorityMask = priority[p];

		for (i = 0; i < sizeof(layers) / sizeof(layers[0]); i++)
		{
			ActiveLayer = layers[i];
			Do_VDP_Only();
			screenBitmap = CreateScreenBitmap(hdc, MD_Screen, height, stride);
			if (screenBitmap != NULL)
			{
				BlitTransform(hdc, screenBitmap, width, height, sidebar, 0, 0,
					RGB(debug_base[colors[i]][0], debug_base[colors[i]][1], debug_base[colors[i]][2]));
				DeleteObject(screenBitmap);
			}
		}
	}
	ActiveLayer = oldActiveLayer;
	PriorityMask = 0x11;
	Do_VDP_Only();
}

static void PlaneCompositePaint_KMod(HWND hwnd, LPDRAWITEMSTRUCT lpdi)
{
	HDC bufferDC;
	HBITMAP bufferBitmap;
	HBITMAP oldBufferBitmap;
	RECT rect;
	int width, height, stride, sidebar;
	int bufferWidth, bufferHeight;

	if (lpdi == NULL || lpdi->CtlID != IDC_PLANECOMPOSITE_MAIN)
		return;

	width = VDP_Reg.Set4 & 0x01 ? 320 : 256;
	height = VDP_Reg.Set2 & 0x08 ? 240 : 224;
	stride = 336 * sizeof(*MD_Screen);
	sidebar = (336 - width) / sizeof(*MD_Screen);
	rect = lpdi->rcItem;
	bufferWidth = rect.right - rect.left;
	bufferHeight = rect.bottom - rect.top;
	if (bufferWidth <= 0 || bufferHeight <= 0)
		return;

	if (!CreateCompositeBuffer(lpdi->hDC, bufferWidth, bufferHeight, &bufferDC, &bufferBitmap, &oldBufferBitmap))
		return;

	rect.left = 0;
	rect.top = 0;
	rect.right = bufferWidth;
	rect.bottom = bufferHeight;
	DrawCompositeBackground(bufferDC, &rect);
	RenderCompositeLayers(bufferDC, width, height, stride, sidebar);
	BitBlt(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, bufferWidth, bufferHeight, bufferDC, 0, 0, SRCCOPY);
	DestroyCompositeBuffer(bufferDC, bufferBitmap, oldBufferBitmap);
}

BOOL CALLBACK PlaneCompositeDialogProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		PlaneCompositeInit_KMod(hwnd);
		break;

	case WM_DRAWITEM:
		PlaneCompositePaint_KMod(hwnd, (LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PLANECOMPOSITE_TRANS:
			show_transparence = (IsDlgButtonChecked(hwnd, IDC_PLANECOMPOSITE_TRANS) == BST_CHECKED);
			InvalidateRect(GetDlgItem(hwnd, IDC_PLANECOMPOSITE_MAIN), NULL, FALSE);
			break;
		default:
			break;
		}
		break;

	case WM_SIZE:
	{
		HWND hexplorer = GetDlgItem(hwnd, IDC_PLANECOMPOSITE_MAIN);
		MoveWindow(hexplorer, 20, 60, LOWORD(lParam) - 40, HIWORD(lParam) - 80, TRUE);
		break;
	}

	case WM_CLOSE:
		CloseWindow_KMod(DMODE_PLANECOMPOSITE);
		break;

	case WM_DESTROY:
		planes_destroy();
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void planes_composite_create(HINSTANCE hInstance, HWND hWndParent)
{
	hPlanesComposite = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DEBUGPLANECOMPOSITE), hWndParent, PlaneCompositeDialogProc);
}

void planes_composite_show(BOOL visibility)
{
	ShowWindow(hPlanesComposite, visibility ? SW_SHOW : SW_HIDE);
}

void planes_composite_update()
{
	if (OpenedWindow_KMod[DMODE_PLANECOMPOSITE-1] == FALSE)	return;

	RedrawWindow(GetDlgItem(hPlanesComposite, IDC_PLANECOMPOSITE_MAIN), NULL, NULL, RDW_INVALIDATE);
}

void planes_composite_reset()
{
	PlaneCompositeInit_KMod(hPlanesComposite);
}
void planes_composite_destroy()
{
	DestroyWindow(hPlanesComposite);
}