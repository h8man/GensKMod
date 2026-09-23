#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

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