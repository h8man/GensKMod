#ifndef KMOD_S68K_H
#define KMOD_S68K_H


#ifdef __cplusplus
extern "C" {
#endif


void s68kdebug_create(HINSTANCE hInstance, HWND hWndParent);
void s68kdebug_show(BOOL visibility);
void s68kdebug_update();
void s68kdebug_reset();
void s68kdebug_destroy();

void s68kdebug_dump();//TODO remove

#ifdef __cplusplus
};
#endif

#endif //KMOD_S68K_H