#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../G_main.h"
#include "../Mem_M68k.h"
#include "../vdp_io.h"

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
	lua_setfield(state, -2, "x32");
	return 1;
}

void lua_bind_emu_register(lua_State *state)
{
	int apiIndex;
	int signaturesIndex;

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
}
