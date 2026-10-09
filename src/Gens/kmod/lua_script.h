#ifndef KMOD_LUA_SCRIPT_H
#define KMOD_LUA_SCRIPT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void lua_script_create(HINSTANCE hInstance, HWND hWndParent);
void lua_script_show(BOOL visibility);
void kmod_lua_output(const char *owner, const char *message);

#ifdef __cplusplus
}
#endif

#endif
