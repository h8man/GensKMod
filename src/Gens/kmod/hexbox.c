#include "hexbox.h"

typedef struct
{
	const BYTE* data;
	SIZE_T size;
	UINT topRow;
	UINT addressLength;
	DWORD addressOffset;
	UINT itemsPerRow;
	UINT dataMode;
	UINT endianness;
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

	UINT itemSize = state && state->dataMode == HEXBOX_MODE_WORD ? 2 : 1;
	SIZE_T rowSize = state ? (SIZE_T)state->itemsPerRow * itemSize : 0;
	UINT rows = state && state->size ? (UINT)((state->size - 1) / rowSize + 1) : 0;
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
		state->addressLength = HEXBOX_DEFAULT_ADDRESS_LENGTH;
		state->itemsPerRow = HEXBOX_DEFAULT_ITEMS_PER_ROW;
		state->dataMode = HEXBOX_MODE_BYTE;
		state->endianness = HEXBOX_ENDIAN_BIG;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
		HexBox_UpdateScroll(hwnd, state);
		return 0;
	case HEXBOX_SET_DATA:
		if (state)
		{
			const BYTE* data = (const BYTE*)lParam;
			SIZE_T size = data ? (SIZE_T)wParam : 0;
			if (state->data != data || state->size != size) state->topRow = 0;
			state->data = data;
			state->size = size;
			HexBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case HEXBOX_SET_LAYOUT:
		if (state)
		{
			UINT addressLength = LOWORD(wParam);
			UINT layoutFlags = HIWORD(wParam);
			UINT itemsPerRow = layoutFlags & HEXBOX_ITEMS_PER_ROW_MASK;
			UINT dataMode = layoutFlags & HEXBOX_MODE_WORD;
			UINT endianness = layoutFlags & HEXBOX_ENDIAN_LITTLE;
			if (!addressLength) addressLength = 1;
			if (addressLength > HEXBOX_MAX_ADDRESS_LENGTH) addressLength = HEXBOX_MAX_ADDRESS_LENGTH;
			if (!itemsPerRow) itemsPerRow = 1;
			if (itemsPerRow > HEXBOX_MAX_ITEMS_PER_ROW) itemsPerRow = HEXBOX_MAX_ITEMS_PER_ROW;
			if (state->itemsPerRow != itemsPerRow || state->dataMode != dataMode) state->topRow = 0;
			state->addressLength = addressLength;
			state->addressOffset = (DWORD)lParam;
			state->itemsPerRow = itemsPerRow;
			state->dataMode = dataMode;
			state->endianness = endianness;
			HexBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
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
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
	{
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

		if (state && state->data)
		{
			static const char digits[] = "0123456789ABCDEF";
			UINT itemSize = state->dataMode == HEXBOX_MODE_WORD ? 2 : 1;
			UINT hexDigitsPerItem = state->dataMode == HEXBOX_MODE_WORD ? 4 : 2;
			UINT hexStride = hexDigitsPerItem + 1;
			SIZE_T rowSize = (SIZE_T)state->itemsPerRow * itemSize;
			int hexLength = (int)(state->itemsPerRow * hexStride - 1);
			for (UINT row = 0, y = 0; y < (UINT)rect.bottom; ++row, y += metrics.tmHeight)
			{
				SIZE_T offset = ((SIZE_T)state->topRow + row) * rowSize;
				if (offset >= state->size) break;
				char address[HEXBOX_MAX_ADDRESS_LENGTH], hex[HEXBOX_MAX_ITEMS_PER_ROW * 5], text[HEXBOX_MAX_ITEMS_PER_ROW * 2];
				SIZE_T count = state->size - offset;
				DWORD addressValue = state->addressOffset + (DWORD)offset;
				if (count > rowSize) count = rowSize;
				for (UINT i = 0; i < state->addressLength; ++i)
					address[i] = digits[(addressValue >> ((state->addressLength - i - 1) * 4)) & 15];
				for (SIZE_T i = 0; i < count; ++i)
				{
					BYTE value = state->data[offset + i];
					text[i] = value >= 32 && value < 127 ? value : '.';
				}
				for (UINT i = 0; i < state->itemsPerRow; ++i)
				{
					UINT available = (UINT)(count > (SIZE_T)i * itemSize ? count - (SIZE_T)i * itemSize : 0);
					UINT hexPos = i * hexStride;
					for (UINT j = 0; j < hexDigitsPerItem; ++j) hex[hexPos + j] = ' ';
					if (state->dataMode == HEXBOX_MODE_BYTE && available)
					{
						BYTE value = state->data[offset + i];
						hex[hexPos] = digits[value >> 4];
						hex[hexPos + 1] = digits[value & 15];
					}
					else if (available >= 2)
					{
						BYTE first = state->data[offset + (SIZE_T)i * 2];
						BYTE second = state->data[offset + (SIZE_T)i * 2 + 1];
						WORD value = state->endianness == HEXBOX_ENDIAN_LITTLE ? (WORD)(first | (second << 8)) : (WORD)((first << 8) | second);
						for (UINT j = 0; j < 4; ++j) hex[hexPos + j] = digits[(value >> ((3 - j) * 4)) & 15];
					}
					else if (available == 1)
					{
						BYTE value = state->data[offset + (SIZE_T)i * itemSize];
						hex[hexPos] = digits[value >> 4];
						hex[hexPos + 1] = digits[value & 15];
					}
					if (i + 1 < state->itemsPerRow) hex[hexPos + hexDigitsPerItem] = ' ';
				}
				TextOutA(dc, 4, y, address, state->addressLength);
				int hexX = 4 + metrics.tmAveCharWidth * (state->addressLength + 2);
				TextOutA(dc, hexX, y, hex, hexLength);
				TextOutA(dc, hexX + metrics.tmAveCharWidth * (hexLength + 3), y, text, (int)count);
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