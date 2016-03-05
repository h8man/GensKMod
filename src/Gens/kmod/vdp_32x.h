#ifndef KMOD_VDP_32X_H
#define KMOD_VDP_32X_H

extern UCHAR pal_KMod;

#ifdef __cplusplus
extern "C" {
#endif

void vdp32x_create(HINSTANCE hInstance, HWND hWndParent);
void vdp32x_show(BOOL visibility);
void vdp32x_update();
void vdp32x_reset();
void vdp32x_destroy();



#ifdef __cplusplus
};
#endif

#endif //KMOD_VDP_32X_H