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
