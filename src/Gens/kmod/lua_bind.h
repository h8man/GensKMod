#ifndef KMOD_LUA_BIND_H
#define KMOD_LUA_BIND_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

void lua_bind_register(lua_State *state);

#ifdef __cplusplus
}
#endif

#endif
