#pragma once

#include <windows.h>

#define DASMBOX_CLASS_NAME L"GensDasmBox"
#define DASMBOX_WINDOW_STYLE (WS_CHILD | WS_VISIBLE)
#define DASMBOX_SET_SOURCE (WM_APP + 1)
#define DASMBOX_GOTO_ADDRESS (WM_APP + 2)
#define DASMBOXN_SCROLL 1
#define DASMBOX_DEFAULT_ADDRESS_LENGTH 8
#define DASMBOX_MAX_ADDRESS_LENGTH 8
#define DASMBOX_TEXT_CAPACITY 256

/* The callback receives source-relative offsets and returns the next source-relative offset. */
typedef BOOL (CALLBACK *DASMBOX_GET_INSTRUCTION)(void* context, DWORD offset, DWORD* nextOffset, LPSTR text, UINT textCapacity);

/* size is the source extent/one-past-end offset; displayed addresses are startAddress + offset. */
typedef struct
{
	DWORD size;
	UINT addressLength;
	DWORD startAddress;
	void* context;
	DASMBOX_GET_INSTRUCTION getInstruction;
} DASMBOX_SOURCE;

/* DASMBOXN_SCROLL reports the topLine scroll position. GOTO_ADDRESS takes an address of offset. */

#ifdef __cplusplus
extern "C" {
#endif

BOOL DasmBox_RegisterClass(HINSTANCE instance);

#ifdef __cplusplus
}
#endif
