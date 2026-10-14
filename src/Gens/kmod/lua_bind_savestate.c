#include <stdio.h>

#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../G_main.h"
#include "../save.h"
#include "../vdp_io.h"

static int LuaBindSavestateSave(lua_State *state)
{
	lua_Integer slot = luaL_checkinteger(state, 1);
	char filename[2048];
	int previousSlot = Current_State;
	int result;

	if (slot < 0 || slot > 9)
		return luaL_argerror(state, 1, "slot must be between 0 and 9");
	if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
		return luaL_error(state, "no emulated system is running");

	Current_State = (int)slot;
	Get_State_File_Name(filename);
	result = Save_State(filename);
	Current_State = previousSlot;
	lua_pushboolean(state, result != 0);
	return 1;
}

static int LuaBindSavestateLoad(lua_State *state)
{
	lua_Integer slot = luaL_checkinteger(state, 1);
	char filename[2048];
	int previousSlot = Current_State;
	int result;

	if (slot < 0 || slot > 9)
		return luaL_argerror(state, 1, "slot must be between 0 and 9");
	if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
		return luaL_error(state, "no emulated system is running");

	Current_State = (int)slot;
	Get_State_File_Name(filename);
	result = Load_State(filename);
	Current_State = previousSlot;
	lua_pushboolean(state, result != 0);
	return 1;
}

void lua_bind_savestate_register(lua_State *state)
{
	int savestateIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 3);
	savestateIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 2);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, savestateIndex, signaturesIndex, "save", LuaBindSavestateSave, "(slot)");
	KMOD_LUA_BIND_FUNCTION(state, savestateIndex, signaturesIndex, "load", LuaBindSavestateLoad, "(slot)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, savestateIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "savestate");
}
