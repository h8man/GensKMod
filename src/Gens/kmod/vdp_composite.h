#ifndef KMOD_VDP_COMPOSITE_H
#define KMOD_VDP_COMPOSITE_H


#ifdef __cplusplus
extern "C" {
#endif


void planes_composite_create(HINSTANCE hInstance, HWND hWndParent);
void planes_composite_show(BOOL visibility);
void planes_composite_update();
void planes_composite_reset();
void planes_composite_destroy();


#ifdef __cplusplus
};
#endif

#endif //KMOD_PLANES_H