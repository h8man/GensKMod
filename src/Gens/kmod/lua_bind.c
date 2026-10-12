#include "lauxlib.h"

#include "lua_bind.h"
#include "lua_script.h"
#include "../gens.h"
#include "../G_main.h"
#include "../Mem_M68k.h"
#include "../vdp_io.h"

#define KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, name, function, signature) \
	do { \
		lua_pushcfunction(state, function); \
		lua_setfield(state, apiIndex, name); \
		lua_pushstring(state, signature); \
		lua_setfield(state, signaturesIndex, name); \
	} while (0)

static int LuaBindFrameCount(lua_State *state)
{
	lua_pushinteger(state, (lua_Integer)Frame_Number);
	return 1;
}

static int LuaBindIsPaused(lua_State *state)
{
	lua_pushboolean(state, Paused != 0);
	return 1;
}

static int LuaBindGetRomSize(lua_State *state)
{
	lua_pushinteger(state, (lua_Integer)Rom_Size);
	return 1;
}

static int LuaBindGetSystems(lua_State *state)
{
	lua_createtable(state, 0, 3);
	lua_pushboolean(state, Genesis_Started != 0);
	lua_setfield(state, -2, "genesis");
	lua_pushboolean(state, SegaCD_Started != 0);
	lua_setfield(state, -2, "sega_cd");
	lua_pushboolean(state, _32X_Started != 0);
	lua_setfield(state, -2, "32x");
	return 1;
}

static int LuaBindLog(lua_State *state)
{
	const char *message = luaL_checkstring(state, 1);
	kmod_lua_output("LUA", message);
	return 0;
}

void lua_bind_register(lua_State *state)
{
	int apiIndex;
	int signaturesIndex;
	int kmodIndex;
	int kmodSignaturesIndex;

	lua_createtable(state, 0, 5);
	apiIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 4);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "framecount", LuaBindFrameCount, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "ispaused", LuaBindIsPaused, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getromsize", LuaBindGetRomSize, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getsystems", LuaBindGetSystems, "()");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, apiIndex, "signatures");
	lua_pop(state, 1);
	lua_pushvalue(state, apiIndex);
	lua_setglobal(state, "gens");
	lua_setglobal(state, "emu");

	lua_createtable(state, 0, 2);
	kmodIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 1);
	kmodSignaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, kmodSignaturesIndex, "log", LuaBindLog, "(message)");
	lua_pushvalue(state, kmodSignaturesIndex);
	lua_setfield(state, kmodIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "kmod");
}
