#include "lauxlib.h"

#include "lua_bind.h"
#include "lua_script.h"

static int LuaBindLog(lua_State *state)
{
	const char *message = luaL_checkstring(state, 1);
	kmod_lua_output("KMOD", message);
	return 0;
}

void lua_bind_kmod_register(lua_State *state)
{
	int kmodIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 2);
	kmodIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 1);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "log", LuaBindLog, "(message)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, kmodIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "kmod");
}
