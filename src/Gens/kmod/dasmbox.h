#pragma once

#include <windows.h>

#define DASMBOX_CLASS_NAME L"GensDasmBox"
#define DASMBOX_WINDOW_STYLE (WS_CHILD | WS_VISIBLE)
#define DASMBOX_SET_SOURCE (WM_APP + 1)
#define DASMBOX_DEFAULT_ADDRESS_LENGTH 8
#define DASMBOX_MAX_ADDRESS_LENGTH 8
#define DASMBOX_TEXT_CAPACITY 256

typedef BOOL (CALLBACK *DASMBOX_GET_INSTRUCTION)(void* context, DWORD address, DWORD* nextAddress, LPSTR text, UINT textCapacity);

typedef struct
{
	DWORD lineCount;
	UINT addressLength;
	DWORD startAddress;
	void* context;
	DASMBOX_GET_INSTRUCTION getInstruction;
} DASMBOX_SOURCE;

#ifdef __cplusplus
extern "C" {
#endif

BOOL DasmBox_RegisterClass(HINSTANCE instance);

#ifdef __cplusplus
}
#endif
