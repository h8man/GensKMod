#ifndef KMOD_LUA_SCRIPT_H
#define KMOD_LUA_SCRIPT_H

#include <windows.h>

#define WM_KMOD_LUA_STATUS (WM_APP + 0x416)
#define WM_KMOD_LUA_CLEAR_OUTPUT (WM_APP + 0x417)

typedef enum LuaScriptStatus
{
	LUA_SCRIPT_STATUS_NO_ROM,
	LUA_SCRIPT_STATUS_IDLE,
	LUA_SCRIPT_STATUS_RUNNING,
	LUA_SCRIPT_STATUS_STOPPING,
	LUA_SCRIPT_STATUS_ABORTING,
	LUA_SCRIPT_STATUS_COMPLETED,
	LUA_SCRIPT_STATUS_ERROR,
	LUA_SCRIPT_STATUS_STOPPED,
	LUA_SCRIPT_STATUS_UNAVAILABLE
} LuaScriptStatus;

#ifdef __cplusplus
extern "C" {
#endif

void lua_script_create(HINSTANCE hInstance, HWND hWndParent);
void lua_script_show(BOOL visibility);
void lua_script_set_status(LuaScriptStatus status);
void kmod_lua_output(const char *owner, const char *message);
void kmod_lua_clear_output(void);

#ifdef __cplusplus
}
#endif

#endif
