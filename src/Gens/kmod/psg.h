#ifndef KMOD_PSG_H
#define KMOD_PSG_H

#ifdef __cplusplus
extern "C" {
#endif

void psg_create(HINSTANCE hInstance, HWND hWndParent);
void psg_show(BOOL visibility);
void psg_update();
void psg_reset();
void psg_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_PSG_H