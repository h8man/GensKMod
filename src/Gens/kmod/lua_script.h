#ifndef KMOD_LUA_SCRIPT_H
#define KMOD_LUA_SCRIPT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void lua_script_create(HINSTANCE hInstance, HWND hWndParent);
void lua_script_show(BOOL visibility);

#ifdef __cplusplus
}
#endif

#endif
