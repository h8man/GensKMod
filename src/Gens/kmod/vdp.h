#ifndef KMOD_VDP_H
#define KMOD_VDP_H

extern UCHAR pal_KMod;

#ifdef __cplusplus
extern "C" {
#endif

void vdpdebug_create(HINSTANCE hInstance, HWND hWndParent);
void vdpdebug_show(BOOL visibility);
void vdpdebug_update();
void vdpdebug_reset();
void vdpdebug_destroy();
BOOL vdpdebug_isMessage(LPMSG lpMsg);


void vdpdebug_drawTile(HDC hDCMain, unsigned short int numTile, WORD x, WORD y, UCHAR pal, UCHAR zoom);
COLORREF vdpdebug_getColor(unsigned int numPal, unsigned int numColor);

#ifdef __cplusplus
};
#endif

#endif //KMOD_VDP_H