#include "hexbox.h"

typedef struct
{
	const BYTE* data;
	SIZE_T size;
	UINT topRow;
} HEXBOX_STATE;

static void HexBox_UpdateScroll(HWND hwnd, HEXBOX_STATE* state)
{
	RECT rect;
	TEXTMETRICA metrics;
	HDC dc = GetDC(hwnd);
	HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	HGDIOBJ oldFont = SelectObject(dc, font);
	GetTextMetricsA(dc, &metrics);
	SelectObject(dc, oldFont);
	ReleaseDC(hwnd, dc);
	GetClientRect(hwnd, &rect);

	UINT rows = state && state->size ? (UINT)((state->size - 1) / 16 + 1) : 0;
	UINT page = (UINT)((rect.bottom - rect.top) / metrics.tmHeight);
	if (!page) page = 1;

	SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS, 0, rows ? (int)rows - 1 : 0, page, state ? (int)state->topRow : 0, 0 };
	UINT position = (UINT)SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	if (state) state->topRow = position;
}

static LRESULT CALLBACK HexBox_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HEXBOX_STATE* state = (HEXBOX_STATE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_CREATE:
		{
			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			if (!(style & WS_VSCROLL) || !(exStyle & WS_EX_CLIENTEDGE))
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, style | WS_VSCROLL);
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_CLIENTEDGE);
				SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			}
		}
		state = (HEXBOX_STATE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
		if (!state)
		{
			HexBox_UpdateScroll(hwnd, NULL);
			return 0;
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
		HexBox_UpdateScroll(hwnd, state);
		return 0;
	case HEXBOX_SET_DATA:
		if (state)
		{
			state->data = (const BYTE*)lParam;
			state->size = state->data ? (SIZE_T)wParam : 0;
			state->topRow = 0;
			HexBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;
	case WM_SIZE:
		HexBox_UpdateScroll(hwnd, state);	
		return 0;
	case WM_VSCROLL:
		if (state)
		{
			SCROLLINFO si;
			ZeroMemory(&si, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			switch (LOWORD(wParam))
			{
			case SB_LINEUP: --si.nPos; break;
			case SB_LINEDOWN: ++si.nPos; break;
			case SB_PAGEUP: si.nPos -= (int)si.nPage; break;
			case SB_PAGEDOWN: si.nPos += (int)si.nPage; break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
			case SB_TOP: si.nPos = 0; break;
			case SB_BOTTOM: si.nPos = si.nMax; break;
			}
			si.fMask = SIF_POS;
			state->topRow = (UINT)SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rect;
		TEXTMETRICA metrics;
		HDC dc = BeginPaint(hwnd, &ps);
		HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		HGDIOBJ oldFont = SelectObject(dc, font);
		GetTextMetricsA(dc, &metrics);
		GetClientRect(hwnd, &rect);
		FillRect(dc, &ps.rcPaint, GetSysColorBrush(COLOR_WINDOW));
		SetBkMode(dc, TRANSPARENT);
		SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));

		if (state && state->data)
		{
			static const char digits[] = "0123456789ABCDEF";
			for (UINT row = 0, y = 0; y < (UINT)rect.bottom; ++row, y += metrics.tmHeight)
			{
				SIZE_T offset = ((SIZE_T)state->topRow + row) * 16;
				if (offset >= state->size) break;
				char address[8], hex[47], text[16];
				SIZE_T count = state->size - offset;
				if (count > 16) count = 16;
				for (int i = 0; i < 8; ++i) address[i] = digits[(offset >> ((7 - i) * 4)) & 15];
				for (int i = 0; i < 16; ++i)
				{
					if ((SIZE_T)i < count)
					{
						BYTE value = state->data[offset + i];
						hex[i * 3] = digits[value >> 4];
						hex[i * 3 + 1] = digits[value & 15];
						text[i] = value >= 32 && value < 127 ? value : '.';
					}
					else
					{
						hex[i * 3] = hex[i * 3 + 1] = ' ';
						text[i] = ' ';
					}
					if (i < 15) hex[i * 3 + 2] = ' ';
				}
				TextOutA(dc, 4, y, address, 8);
				TextOutA(dc, 4 + metrics.tmAveCharWidth * 10, y, hex, 47);
				TextOutA(dc, 4 + metrics.tmAveCharWidth * 60, y, text, (int)count);
			}
		}

		SelectObject(dc, oldFont);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_DESTROY:
		if (state)
		{
			HeapFree(GetProcessHeap(), 0, state);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		}
		return 0;
	}

	return DefWindowProcW(hwnd, message, wParam, lParam);
}

BOOL HexBox_RegisterClass(HINSTANCE instance)
{
	WNDCLASSEXW wc = { sizeof(wc) };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = HexBox_WndProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = HEXBOX_CLASS_NAME;
	return RegisterClassExW(&wc) != 0;
}