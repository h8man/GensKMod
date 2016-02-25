#ifndef KMOD_Z80_H
#define KMOD_Z80_H


#ifdef __cplusplus
extern "C" {
#endif

void z80debug_create(HINSTANCE hInstance, HWND hWndParent);
void z80debug_show(BOOL visibility);
void z80debug_update();
void z80debug_reset();
void z80debug_destroy();


void z80debug_dump();//TODO remove

#ifdef __cplusplus
};
#endif

#endif //KMOD_Z80_H