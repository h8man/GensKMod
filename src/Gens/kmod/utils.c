#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/// for all memory views
void Hexview( unsigned char *addr, unsigned char *dest)
{
	wsprintf(dest, "%.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
}


void Ansiview( unsigned char *addr, unsigned char *dest)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		if (addr[i] == 0)
			wsprintf(dest+i, "%c", '.');
		else
			wsprintf(dest+i, "%c", addr[i]);
	}
}

static LRESULT CALLBACK RegisterTextSubclassProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldProc = (WNDPROC)GetProp(hWnd, L"OldRegisterTextProc");
	char* text = (char*)GetProp(hWnd, L"RegisterText");
	if (!oldProc)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_ERASEBKGND:
		return TRUE;

	case WM_SETTEXT:
		{
			const char* newText = (const char*)lParam;
			char* replacement = NULL;
			if (newText)
			{
				size_t length = strlen(newText) + 1;
				replacement = (char*)malloc(length);
				if (!replacement)
					return FALSE;
				memcpy(replacement, newText, length);
			}
			if (replacement && !SetProp(hWnd, L"RegisterText", replacement))
			{
				free(replacement);
				return FALSE;
			}
			if (!replacement)
				RemoveProp(hWnd, L"RegisterText");
			free(text);
			InvalidateRect(hWnd, NULL, FALSE);
			return TRUE;
		}

	case WM_GETTEXT:
		if (wParam && lParam)
		{
			UINT count = (UINT)wParam - 1;
			UINT length = text ? (UINT)strlen(text) : 0;
			if (count > length)
				count = length;
			if (count)
				memcpy((char*)lParam, text, count);
			((char*)lParam)[count] = 0;
			return count;
		}
		return 0;

	case WM_GETTEXTLENGTH:
		return text ? (LRESULT)strlen(text) : 0;

	case WM_GETFONT:
		return CallWindowProc(oldProc, hWnd, message, wParam, lParam);

	case WM_SETFONT:
		{
			LRESULT result = CallWindowProc(oldProc, hWnd, message, wParam, lParam);
			if (lParam)
				InvalidateRect(hWnd, NULL, FALSE);
			return result;
		}

	case WM_PAINT:
		{
			PAINTSTRUCT paint;
			RECT clientRect;
			HDC targetDC = BeginPaint(hWnd, &paint);
			HDC bufferDC;
			HBITMAP bufferBitmap;
			HGDIOBJ oldBitmap;
			HFONT font = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
			HGDIOBJ oldFont;
			int width, height;

			GetClientRect(hWnd, &clientRect);
			width = clientRect.right;
			height = clientRect.bottom;
			bufferDC = width > 0 && height > 0 ? CreateCompatibleDC(targetDC) : NULL;
			bufferBitmap = bufferDC ? CreateCompatibleBitmap(targetDC, width, height) : NULL;
			oldBitmap = bufferBitmap ? SelectObject(bufferDC, bufferBitmap) : NULL;
			if (oldBitmap && oldBitmap != HGDI_ERROR)
			{
				FillRect(bufferDC, &clientRect, GetSysColorBrush(COLOR_BTNFACE));
				oldFont = font ? SelectObject(bufferDC, font) : NULL;
				SetBkMode(bufferDC, TRANSPARENT);
				SetTextColor(bufferDC, GetSysColor(COLOR_WINDOWTEXT));
			DrawText(bufferDC, text ? text : "", -1, &clientRect,
					DT_TOP | DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
				if (oldFont)
					SelectObject(bufferDC, oldFont);
				BitBlt(targetDC, 0, 0, width, height, bufferDC, 0, 0, SRCCOPY);
				SelectObject(bufferDC, oldBitmap);
			}
			else
				FillRect(targetDC, &clientRect, GetSysColorBrush(COLOR_BTNFACE));

			if (bufferBitmap)
				DeleteObject(bufferBitmap);
			if (bufferDC)
				DeleteDC(bufferDC);
			EndPaint(hWnd, &paint);
			return 0;
		}

	case WM_NCDESTROY:
		{
			LRESULT result = CallWindowProc(oldProc, hWnd, message, wParam, lParam);
			free(GetProp(hWnd, L"RegisterText"));
			RemoveProp(hWnd, L"RegisterText");
			RemoveProp(hWnd, L"OldRegisterTextProc");
			return result;
		}
	}

	return CallWindowProc(oldProc, hWnd, message, wParam, lParam);
}

void SubclassRegisterText(HWND hDlg, int controlID)
{
	HWND control = GetDlgItem(hDlg, controlID);
	WNDPROC oldProc;
	if (!control || GetProp(control, L"OldRegisterTextProc"))
		return;

	oldProc = (WNDPROC)GetWindowLongPtr(control, GWLP_WNDPROC);
	if (oldProc && SetProp(control, L"OldRegisterTextProc", (HANDLE)oldProc))
	{
		SetWindowLongPtr(control, GWLP_WNDPROC, (LONG_PTR)RegisterTextSubclassProc);
		if ((WNDPROC)GetWindowLongPtr(control, GWLP_WNDPROC) != RegisterTextSubclassProc)
		{
			RemoveProp(control, L"OldRegisterTextProc");
			return;
		}
		SendMessage(control, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), FALSE);
	}
}

int CopyToClipboard(int Type, unsigned char* Buffer, size_t buflen, BOOL clear) // feos added this
{
	HGLOBAL hResult;
	if (!OpenClipboard(NULL)) return 0;
	if (clear)
	{
		if (!EmptyClipboard()) return 0;
	}

	hResult = GlobalAlloc(GMEM_MOVEABLE, buflen);
	if (hResult == NULL) return 0;

	memcpy(GlobalLock(hResult), Buffer, buflen);
	GlobalUnlock(hResult);

	if (SetClipboardData(Type, hResult) == NULL)
	{
		CloseClipboard();
		GlobalFree(hResult);
		return 0;
	}

	CloseClipboard();
	GlobalFree(hResult);
	return 1;
}

LRESULT CALLBACK ReusableEditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Retrieve the unique original window procedure attached to this specific control
	WNDPROC oldProc = (WNDPROC)GetProp(hWnd, L"OldEditProc");
	if (!oldProc) return DefWindowProc(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
	//case WM_GETDLGCODE:
	//	return DLGC_WANTALLKEYS;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			HWND hParent = GetParent(hWnd);
			// GetDlgCtrlID dynamically looks up the correct ID (e.g., IDC_68K_JUMP_TO_INPUT or IDC_68K_JUMP_TO)
			int controlID = GetDlgCtrlID(hWnd);

			// Send custom notification code 1 to the parent
			SendMessage(hParent, WM_COMMAND, MAKEWPARAM(controlID, EN_MAXTEXT), (LPARAM)hWnd);
			return 0;
		}
		break;

	case WM_NCDESTROY:
		// Clean up the property when the window is destroyed to prevent memory leaks
		RemoveProp(hWnd, L"OldEditProc");
		break;
	}

	return CallWindowProc(oldProc, hWnd, uMsg, wParam, lParam);
}

void SubclassEditMaxText(HWND hDlg, int controlID)
{
	HWND hEdit = GetDlgItem(hDlg, controlID);
	if (hEdit)
	{
		// 1. Save the original procedure as a window property
		WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)ReusableEditSubclassProc);

		// 2. Set the property so the subclass proc can find it
		SetProp(hEdit, L"OldEditProc", (HANDLE)oldProc);
	}
}

