#pragma once

#include <windows.h>

#define HEXBOX_CLASS_NAME L"GensHexBox"
#define HEXBOX_WINDOW_STYLE (WS_CHILD | WS_VISIBLE)
/* Send with wParam = byte count and lParam = caller-owned BYTE* buffer. */
#define HEXBOX_SET_DATA (WM_APP + 1)

#ifdef __cplusplus
extern "C" {
#endif

BOOL HexBox_RegisterClass(HINSTANCE instance);

#ifdef __cplusplus
}
#endif
