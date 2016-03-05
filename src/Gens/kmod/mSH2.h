#ifndef KMOD_32X_MSH2_H
#define KMOD_32X_MSH2_H

#ifdef __cplusplus
extern "C" {
#endif

void mSH2_create(HINSTANCE hInstance, HWND hWndParent);
void mSH2_show(BOOL visibility);
void mSH2_update();
void mSH2_reset();
void mSH2_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_32X_MSH2_H