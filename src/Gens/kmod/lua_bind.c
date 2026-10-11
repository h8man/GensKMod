#include "lauxlib.h"

#include "lua_bind.h"
#include "lua_script.h"

static int LuaBindLog(lua_State *state)
{
	const char *message = luaL_checkstring(state, 1);
	kmod_lua_output("LUA", message);
	return 0;
}

void lua_bind_register(lua_State *state)
{
	lua_createtable(state, 0, 1);
	lua_pushcfunction(state, LuaBindLog);
	lua_setfield(state, -2, "log");
	lua_setglobal(state, "kmod");
}
