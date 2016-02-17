#ifndef KMOD_LAYERS_H
#define KMOD_LAYERS_H


#ifdef __cplusplus
extern "C" {
#endif


void layers_create(HINSTANCE hInstance, HWND hWndParent);
void layers_show(BOOL visibility);
void layers_update();
void layers_reset();
void layers_destroy();


#ifdef __cplusplus
};
#endif

#endif //KMOD_LAYERS_H