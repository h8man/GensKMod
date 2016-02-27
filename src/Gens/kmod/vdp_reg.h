#ifndef KMOD_VDP_REG_H
#define KMOD_VDP_REG_H

extern UCHAR pal_KMod;

#ifdef __cplusplus
extern "C" {
#endif

void vdpreg_create(HINSTANCE hInstance, HWND hWndParent);
void vdpreg_show(BOOL visibility);
void vdpreg_update();
void vdpreg_reset();
void vdpreg_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_VDP_REG_H