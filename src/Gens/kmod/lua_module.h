#ifndef KMOD_LUA_MODULE_H
#define KMOD_LUA_MODULE_H

#include <windows.h>

typedef enum LuaModuleState
{
	LUA_MODULE_STATE_CLOSED,
	LUA_MODULE_STATE_READY,
	LUA_MODULE_STATE_RUNNING,
	LUA_MODULE_STATE_STOPPING
} LuaModuleState;

#ifdef __cplusplus
extern "C" {
#endif

BOOL lua_module_init(void);
void lua_module_reset(void);
void lua_module_close(void);
LuaModuleState lua_module_get_state(void);
BOOL lua_module_get_current_script(char *buffer, DWORD bufferSize);
BOOL lua_module_run(const char *buff, const char *tag);
BOOL lua_module_stop(void);
void lua_module_update(void);

#ifdef __cplusplus
}
#endif

#endif
