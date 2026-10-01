#pragma once

#include <windows.h>

#define HEXBOX_CLASS_NAME L"GensHexBox"
#define HEXBOX_WINDOW_STYLE (WS_CHILD | WS_VISIBLE)
#define HEXBOX_SET_DATA (WM_APP + 1)
#define HEXBOX_SET_LAYOUT (WM_APP + 2)
#define HEXBOX_GOTO_ADDRESS (WM_APP + 3)
#define HEXBOXN_SCROLL 1
#define HEXBOX_STYLE_EDITABLE 0x00008000L
#define HEXBOXN_EDIT (0U - 1000U)
#define HEXBOX_LAYOUT_WPARAM(addressLength, itemsPerRow, flags) \
	((WPARAM)MAKELONG((addressLength), (((itemsPerRow) & HEXBOX_ITEMS_PER_ROW_MASK) | (flags))))
#define HEXBOX_ITEMS_PER_ROW_MASK 0x007F
#define HEXBOX_MODE_BYTE 0x0000
#define HEXBOX_MODE_WORD 0x0100
#define HEXBOX_ENDIAN_BIG 0x0000
#define HEXBOX_ENDIAN_LITTLE 0x0200
#define HEXBOX_DEFAULT_ADDRESS_LENGTH 8
#define HEXBOX_DEFAULT_ITEMS_PER_ROW 16
#define HEXBOX_DEFAULT_DATA_MODE HEXBOX_MODE_BYTE
#define HEXBOX_DEFAULT_ENDIANNESS HEXBOX_ENDIAN_BIG
#define HEXBOX_MAX_ADDRESS_LENGTH 8
#define HEXBOX_MAX_ITEMS_PER_ROW 64
/*
 * Offset is source-relative and ranges from zero through the data size; size is the one-past-end boundary.
 * The displayed address is startAddress + offset.
 * SET_DATA: wParam=byte count, lParam=BYTE*. SET_LAYOUT: wParam=HEXBOX_LAYOUT_WPARAM(...), lParam=startAddress.
 * GOTO_ADDRESS takes an address or offset; HEXBOXN_SCROLL reports the topRow scroll position.
 */

typedef struct
{
	NMHDR hdr;
	DWORD offset;
	DWORD address;
	UINT itemSize;
	WORD value;
} HEXBOX_EDIT_NOTIFICATION;

#ifdef __cplusplus
extern "C" {
#endif

BOOL HexBox_RegisterClass(HINSTANCE instance);

#ifdef __cplusplus
}
#endif
