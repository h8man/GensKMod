#ifndef KMOD_CDC_H
#define KMOD_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

void cdcdebug_create(HINSTANCE hInstance, HWND hWndParent);
void cdcdebug_show(BOOL visibility);
void cdcdebug_update();
void cdcdebug_reset();
void cdcdebug_destroy();

#ifdef __cplusplus
};
#endif

#endif //KMOD_CDC_H