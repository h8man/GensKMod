#ifndef KMOD_WATCHERS_H
#define KMOD_WATCHERS_H


#ifdef __cplusplus
extern "C" {
#endif


void watchers_create(HINSTANCE hInstance, HWND hWndParent);
void watchers_show(BOOL visibility);
void watchers_update();
void watchers_reset();
void watchers_destroy();


#ifdef __cplusplus
};
#endif

#endif //KMOD_WATCHERS_H