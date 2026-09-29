#include "dasmbox.h"

#include <string.h>

typedef struct
{
	DASMBOX_SOURCE source;
	UINT topLine;
} DASMBOX_STATE;

static void DasmBox_UpdateScroll(HWND hwnd, DASMBOX_STATE* state)
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

	UINT page = (UINT)((rect.bottom - rect.top) / metrics.tmHeight);
	if (!page) page = 1;
	DWORD lineCount = state ? state->source.lineCount : 0;
	SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS, 0, lineCount ? (int)lineCount - 1 : 0, page, state ? (int)state->topLine : 0, 0 };
	if (state) state->topLine = (UINT)SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	else SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

static LRESULT CALLBACK DasmBox_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DASMBOX_STATE* state = (DASMBOX_STATE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
		state = (DASMBOX_STATE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
		if (!state)
		{
			DasmBox_UpdateScroll(hwnd, NULL);
			return 0;
		}
		state->source.addressLength = DASMBOX_DEFAULT_ADDRESS_LENGTH;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
		DasmBox_UpdateScroll(hwnd, state);
		return 0;
	case DASMBOX_SET_SOURCE:
		if (state)
		{
			if (lParam) state->source = *(const DASMBOX_SOURCE*)lParam;
			else ZeroMemory(&state->source, sizeof(state->source));
			if (!state->source.addressLength) state->source.addressLength = DASMBOX_DEFAULT_ADDRESS_LENGTH;
			if (state->source.addressLength > DASMBOX_MAX_ADDRESS_LENGTH) state->source.addressLength = DASMBOX_MAX_ADDRESS_LENGTH;
			if (!state->source.getInstruction) state->source.lineCount = 0;
			if (state->source.lineCount > 0x7FFFFFFF) state->source.lineCount = 0x7FFFFFFF;

			DasmBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case DASMBOX_GOTO_ADDRESS:
		if (state)
		{
			state->topLine = (UINT)wParam;
			DasmBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case WM_SIZE:
		DasmBox_UpdateScroll(hwnd, state);
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
			state->topLine = (UINT)SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
			InvalidateRect(hwnd, NULL, FALSE);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), DASMBOXN_SCROLL), (LPARAM)state->topLine);
		}
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
	{
		static const char digits[] = "0123456789ABCDEF";
		PAINTSTRUCT ps;
		RECT rect;
		TEXTMETRICA metrics;
		HDC paintDc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		HDC bufferDc = CreateCompatibleDC(paintDc);
		HBITMAP bitmap = bufferDc ? CreateCompatibleBitmap(paintDc, rect.right, rect.bottom) : NULL;
		HGDIOBJ oldBitmap = bitmap ? SelectObject(bufferDc, bitmap) : NULL;
		HDC dc = oldBitmap ? bufferDc : paintDc;
		HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
		HGDIOBJ oldFont = SelectObject(dc, font);
		GetTextMetricsA(dc, &metrics);
		FillRect(dc, &rect, GetSysColorBrush(COLOR_WINDOW));
		SetBkMode(dc, TRANSPARENT);
		SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));

		if (state && state->source.getInstruction)
		{
			DWORD address = state->topLine;
			for (UINT row = 0, y = 0; y < (UINT)rect.bottom; ++row, y += metrics.tmHeight)
			{
				DWORD line = state->topLine + row;
				DWORD instructionAddress = address;
				DWORD nextAddress;
				char text[DASMBOX_TEXT_CAPACITY];
				text[0] = 0;
				text[DASMBOX_TEXT_CAPACITY - 1] = 0;

				char addressText[DASMBOX_MAX_ADDRESS_LENGTH];
				if (!state->source.getInstruction(state->source.context, instructionAddress, &nextAddress, text, DASMBOX_TEXT_CAPACITY))
				{
					break;
				}
				address = nextAddress;
				for (UINT i = 0; i < state->source.addressLength; ++i)
					addressText[i] = digits[(instructionAddress >> ((state->source.addressLength - i - 1) * 4)) & 15];
				TextOutA(dc, 4, y, addressText, state->source.addressLength);
				TextOutA(dc, 4 + metrics.tmAveCharWidth * (state->source.addressLength + 3), y, text, lstrlenA(text));
			}
		}

		SelectObject(dc, oldFont);
		if (oldBitmap)
		{
			BitBlt(paintDc, 0, 0, rect.right, rect.bottom, dc, 0, 0, SRCCOPY);
			SelectObject(bufferDc, oldBitmap);
		}
		if (bitmap) DeleteObject(bitmap);
		if (bufferDc) DeleteDC(bufferDc);
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

BOOL DasmBox_RegisterClass(HINSTANCE instance)
{
	WNDCLASSEXW wc = { sizeof(wc) };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DasmBox_WndProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = DASMBOX_CLASS_NAME;
	return RegisterClassExW(&wc) != 0;
}
