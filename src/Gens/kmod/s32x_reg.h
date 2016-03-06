#ifndef KMOD_32X_REG_H
#define KMOD_32X_REG_H


#ifdef __cplusplus
extern "C" {
#endif

void s32xreg_create(HINSTANCE hInstance, HWND hWndParent);
void s32xreg_show(BOOL visibility);
void s32xreg_update();
void s32xreg_reset();
void s32xreg_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_32X_REG_H