#ifndef KMOD_PLANES_H
#define KMOD_PLANES_H


#ifdef __cplusplus
extern "C" {
#endif


void planes_create(HINSTANCE hInstance, HWND hWndParent);
void planes_show(BOOL visibility);
void planes_update();
void planes_reset();
void planes_destroy();


#ifdef __cplusplus
};
#endif

#endif //KMOD_PLANES_H