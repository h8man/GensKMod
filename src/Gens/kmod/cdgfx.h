#ifndef KMOD_CDGFX_H
#define KMOD_CDGFX_H

#ifdef __cplusplus
extern "C" {
#endif

void cdgfx_create(HINSTANCE hInstance, HWND hWndParent);
void cdgfx_show(BOOL visibility);
void cdgfx_update();
void cdgfx_reset();
void cdgfx_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_CDGFX_H