#ifndef KMOD_CD_REG_H
#define KMOD_CD_REG_H

#ifdef __cplusplus
extern "C" {
#endif

void cdreg_create(HINSTANCE hInstance, HWND hWndParent);
void cdreg_show(BOOL visibility);
void cdreg_update();
void cdreg_reset();
void cdreg_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_CD_REG_H