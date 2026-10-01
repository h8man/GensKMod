#include "hexbox.h"
#include <ctype.h>
#include <stdlib.h>

#define COLUMN_SPACING 2
#define HEXBOX_ID_EDIT 1001
#define HEXBOX_ID_OK 1002
#define HEXBOX_ID_CANCEL 1003

typedef struct
{
	HWND editBox;
	HWND okButton;
	HWND cancelButton;
	WNDPROC editWindowProc;
	BOOL editing;
	SIZE_T offset;
	DWORD address;
	UINT itemSize;
} HEXBOX_EDIT_STATE;

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
	HEXBOX_EDIT_STATE edit;
} HEXBOX_STATE;

static LRESULT CALLBACK HexBox_EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND parent = GetParent(hwnd);
	HEXBOX_STATE* state = (HEXBOX_STATE*)GetWindowLongPtr(parent, GWLP_USERDATA);
	WNDPROC editWindowProc = state ? state->edit.editWindowProc : NULL;
	if (!editWindowProc) return DefWindowProc(hwnd, message, wParam, lParam);
	//prevent closing dialogs on enter/escape while editing
	if (message == WM_GETDLGCODE)
	{
		LRESULT result = CallWindowProc(editWindowProc, hwnd, message, wParam, lParam);
		if (wParam == VK_RETURN || wParam == VK_ESCAPE) result |= DLGC_WANTMESSAGE;
		return result;
	}
	//handle enter/escape to send notifications to parent window
	if (message == WM_KEYDOWN && (wParam == VK_RETURN || wParam == VK_ESCAPE))
	{
		UINT command = wParam == VK_RETURN ? HEXBOX_ID_OK : HEXBOX_ID_CANCEL;
		SendMessage(parent, WM_COMMAND, MAKEWPARAM(command, BN_CLICKED), (LPARAM)hwnd);
		return 0;
	}
	return CallWindowProc(editWindowProc, hwnd, message, wParam, lParam);
}

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

static BOOL HexBox_CreateEditor(HWND hwnd, HEXBOX_STATE* state)
{
	HINSTANCE instance;

	instance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
	state->edit.editBox = CreateWindowEx(
		0, "EDIT", "", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_CENTER,
		0, 0, 112, 22, hwnd, (HMENU)(INT_PTR)HEXBOX_ID_EDIT, instance, NULL);
	state->edit.okButton = CreateWindowEx(
		0, "BUTTON", "Ok", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
		0, 0, 32, 20, hwnd, (HMENU)(INT_PTR)HEXBOX_ID_OK, instance, NULL);
	state->edit.cancelButton = CreateWindowEx(
		0, "BUTTON", "Dis", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
		0, 0, 32, 20, hwnd, (HMENU)(INT_PTR)HEXBOX_ID_CANCEL, instance, NULL);
	if (!state->edit.editBox || !state->edit.okButton || !state->edit.cancelButton)
	{
		if (state->edit.editBox) DestroyWindow(state->edit.editBox);
		if (state->edit.okButton) DestroyWindow(state->edit.okButton);
		if (state->edit.cancelButton) DestroyWindow(state->edit.cancelButton);
		state->edit.editBox = NULL;
		state->edit.okButton = NULL;
		state->edit.cancelButton = NULL;
		return FALSE;
	}
	SendMessage(state->edit.editBox, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), FALSE);
	SendMessage(state->edit.okButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
	SendMessage(state->edit.cancelButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), FALSE);
	state->edit.editWindowProc = (WNDPROC)SetWindowLongPtr(state->edit.editBox, GWLP_WNDPROC, (LONG_PTR)HexBox_EditProc);
	return TRUE;
}

static void HexBox_HideEditor(HEXBOX_STATE* state)
{
	if (!state || !state->edit.editing) return;
	ShowWindow(state->edit.editBox, SW_HIDE);
	ShowWindow(state->edit.okButton, SW_HIDE);
	ShowWindow(state->edit.cancelButton, SW_HIDE);
	state->edit.editing = FALSE;
}

static void HexBox_ShowEditor(HWND hwnd, HEXBOX_STATE* state, int mouseX, int mouseY)
{
	TEXTMETRICA metrics;
	RECT rect;
	HDC dc;
	HGDIOBJ oldFont;
	UINT itemSize, digitsPerItem, stride, column;
	int hexStart, charWidth, editorX, editorY, editorWidth;
	SIZE_T rowSize, row, offset;
	WORD value;
	char text[5];

	if (!state || !state->edit.editBox || !state->data) return;

	dc = GetDC(hwnd);
	oldFont = SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetricsA(dc, &metrics);
	SelectObject(dc, oldFont);
	ReleaseDC(hwnd, dc);
	GetClientRect(hwnd, &rect);

	itemSize = state->dataMode == HEXBOX_MODE_WORD ? 2 : 1;
	digitsPerItem = itemSize * 2;
	stride = digitsPerItem + 1;
	charWidth = metrics.tmAveCharWidth;
	hexStart = 4 + charWidth * (state->addressLength + COLUMN_SPACING);
	if (mouseX < hexStart || mouseY < 0 || !metrics.tmHeight || !charWidth) return;
	column = (UINT)((mouseX - hexStart) / (charWidth * (int)stride));
	if (column >= state->itemsPerRow ||
		(mouseX - hexStart) % (charWidth * (int)stride) >= charWidth * (int)digitsPerItem)
		return;

	rowSize = (SIZE_T)state->itemsPerRow * itemSize;
	row = (SIZE_T)state->topRow + (UINT)(mouseY / metrics.tmHeight);
	if (row > ((SIZE_T)-1 - (SIZE_T)column * itemSize) / rowSize) return;
	offset = row * rowSize + (SIZE_T)column * itemSize;
	if (offset >= state->size || (itemSize == 2 && state->size - offset < 2)) return;

	if (itemSize == 1)
		value = state->data[offset];
	else if (state->endianness == HEXBOX_ENDIAN_LITTLE)
		value = (WORD)(state->data[offset] | (state->data[offset + 1] << 8));
	else
		value = (WORD)((state->data[offset] << 8) | state->data[offset + 1]);

	state->edit.offset = offset;
	state->edit.address = state->addressOffset + (DWORD)offset;
	state->edit.itemSize = itemSize;
	state->edit.editing = TRUE;
	wsprintfA(text, itemSize == 1 ? "%02X" : "%04X", value);
	SetWindowText(state->edit.editBox, text);
	SendMessage(state->edit.editBox, EM_SETLIMITTEXT, digitsPerItem, 0);

	editorY = (int)((UINT)(mouseY / metrics.tmHeight) * metrics.tmHeight);
	editorX = hexStart + (int)column * charWidth * (int)stride;
	editorWidth = (int)(digitsPerItem+1) * charWidth; //+1 for caret

	SetWindowPos(state->edit.editBox, HWND_TOP, editorX, editorY, editorWidth, 14, SWP_SHOWWINDOW);
	SetWindowPos(state->edit.okButton, HWND_TOP, editorX, editorY + 16, 32, 20, SWP_SHOWWINDOW);
	SetWindowPos(state->edit.cancelButton, HWND_TOP, editorX + 35, editorY + 16, 32, 20, SWP_SHOWWINDOW);
	SetFocus(state->edit.editBox);
	SendMessage(state->edit.editBox, EM_SETSEL, 0, -1);
}

static void HexBox_CommitEditor(HWND hwnd, HEXBOX_STATE* state)
{
	char text[5];
	char* end;
	int length;
	UINT digits;
	WORD value;
	unsigned long parsed;
	HEXBOX_EDIT_NOTIFICATION notification;
	HWND parent;

	if (!state || !state->edit.editing) return;
	digits = state->edit.itemSize * 2;
	length = GetWindowText(state->edit.editBox, text, (int)(sizeof(text) / sizeof(text[0])));
	if (length != (int)digits || !isxdigit((unsigned char)text[0]) ||
		(text[0] == '0' && (text[1] == 'x' || text[1] == 'X')))
		return;
	parsed = strtoul(text, &end, 16);
	if (end != text + length) return;
	value = (WORD)parsed;

	ZeroMemory(&notification, sizeof(notification));
	notification.hdr.hwndFrom = hwnd;
	notification.hdr.idFrom = (UINT_PTR)GetDlgCtrlID(hwnd);
	notification.hdr.code = HEXBOXN_EDIT;
	notification.address = state->edit.address;
	notification.itemSize = state->edit.itemSize;
	notification.value = value;
	parent = GetParent(hwnd);
	HexBox_HideEditor(state);
	InvalidateRect(hwnd, NULL, FALSE);
	SetFocus(hwnd);
	SendMessage(parent, WM_NOTIFY, notification.hdr.idFrom, (LPARAM)&notification);
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
			if (!(style & WS_VSCROLL) || !(style & WS_CLIPCHILDREN) || !(exStyle & WS_EX_CLIENTEDGE))
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, style | WS_VSCROLL | WS_CLIPCHILDREN);
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
		HexBox_CreateEditor(hwnd, state);
		HexBox_UpdateScroll(hwnd, state);
		return 0;
	case HEXBOX_SET_DATA:
		if (state)
		{
			const BYTE* data = (const BYTE*)lParam;
			SIZE_T size = data ? (SIZE_T)wParam : 0;
			if (size < state->size) HexBox_HideEditor(state);
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
			if (state->itemsPerRow != itemsPerRow || state->dataMode != dataMode ||
				state->endianness != endianness || state->addressOffset != (DWORD)lParam)
				HexBox_HideEditor(state);
			state->addressLength = addressLength;
			state->addressOffset = (DWORD)lParam;
			state->itemsPerRow = itemsPerRow;
			state->dataMode = dataMode;
			state->endianness = endianness;
			HexBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case HEXBOX_GOTO_ADDRESS:
		if (state)
		{
			UINT itemSize = state->dataMode == HEXBOX_MODE_WORD ? 2 : 1;
			SIZE_T rowSize = (SIZE_T)state->itemsPerRow * itemSize;
			DWORD address = (DWORD)wParam;
			SIZE_T offset = address >= state->addressOffset ? address - state->addressOffset : address;
			state->topRow = (UINT)(offset / rowSize);
			HexBox_UpdateScroll(hwnd, state);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		if (state && (GetWindowLongPtr(hwnd, GWL_STYLE) & HEXBOX_STYLE_EDITABLE))
			HexBox_ShowEditor(hwnd, state, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
		return 0;
	case WM_SIZE:
		HexBox_HideEditor(state);
		HexBox_UpdateScroll(hwnd, state);	
		return 0;
	case WM_VSCROLL:
		if (state)
		{
			HexBox_HideEditor(state);
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
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), HEXBOXN_SCROLL), (LPARAM)state->topRow);
		}
		return 0;
	case WM_COMMAND:
		if (state && state->edit.editing && HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == HEXBOX_ID_OK)
				HexBox_CommitEditor(hwnd, state);
			else if (LOWORD(wParam) == HEXBOX_ID_CANCEL)
			{
				HexBox_HideEditor(state);
				SetFocus(hwnd);
			}
			return 0;
		}
		break;
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
					SIZE_T textOffset = i;
					BOOL flip = state->dataMode == HEXBOX_MODE_WORD && state->endianness == HEXBOX_ENDIAN_LITTLE && (i ^ 1) < count;
					if (flip) textOffset = i ^ 1;
					BYTE value = state->data[offset + textOffset];
					//text[i] = value >= 32 && value < 127 ? value : '.';
					text[i] = value ? value : '.';
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
				int posX = 4;
				TextOutA(dc, posX, y, address, state->addressLength);
				posX += metrics.tmAveCharWidth * (state->addressLength + COLUMN_SPACING);
				TextOutA(dc, posX, y, hex, hexLength);
				posX += metrics.tmAveCharWidth * (hexLength + COLUMN_SPACING);
				TextOutA(dc, posX, y, text, (int)count);
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
			if (state->edit.editBox && state->edit.editWindowProc)
				SetWindowLongPtr(state->edit.editBox, GWLP_WNDPROC, (LONG_PTR)state->edit.editWindowProc);
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
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = HexBox_WndProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = HEXBOX_CLASS_NAME;
	return RegisterClassExW(&wc) != 0;
}