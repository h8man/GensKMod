#ifndef KMOD_VDP_SPRITES_H
#define KMOD_VDP_SPRITES_H

#ifdef __cplusplus
extern "C" {
#endif

void sprites_create(HINSTANCE hInstance, HWND hWndParent);
void sprites_show(BOOL visibility);
void sprites_update();
void sprites_reset();
void sprites_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_VDP_SPRITES_H