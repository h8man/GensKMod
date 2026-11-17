#ifndef KMOD_LUA_BIND_H
#define KMOD_LUA_BIND_H

#include "lua.h"

typedef enum KmodLuaRequirement
{
	KMOD_LUA_REQUIREMENT_NONE,
	KMOD_LUA_REQUIREMENT_SYSTEM,
	KMOD_LUA_REQUIREMENT_GENESIS
} KmodLuaRequirement;

#ifdef __cplusplus
extern "C" {
#endif

void lua_bind_register(lua_State *state);
int lua_bind_dispatch(lua_State *state);
void lua_bind_emu_register(lua_State *state);
void lua_bind_kmod_register(lua_State *state);
void lua_bind_memory_register(lua_State *state);
void lua_bind_joypad_register(lua_State *state);
void lua_bind_savestate_register(lua_State *state);
void lua_bind_palette_register(lua_State *state);
void lua_bind_tile_register(lua_State *state);
void lua_bind_vdp_register(lua_State *state);

#define KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, name, function, signature) \
	do { \
		lua_pushcfunction(state, function); \
		lua_setfield(state, apiIndex, name); \
		lua_pushstring(state, signature); \
		lua_setfield(state, signaturesIndex, name); \
	} while (0)

#define KMOD_LUA_BIND_FUNCTION_REQUIRES(state, apiIndex, signaturesIndex, name, function, signature, requirement) \
	do { \
		lua_pushcfunction(state, function); \
		lua_pushinteger(state, requirement); \
		lua_pushcclosure(state, lua_bind_dispatch, 2); \
		lua_setfield(state, apiIndex, name); \
		lua_pushstring(state, signature); \
		lua_setfield(state, signaturesIndex, name); \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
