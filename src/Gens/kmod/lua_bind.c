#include "lua_bind.h"
#include "lauxlib.h"
#include "../gens.h"
#include "../vdp_io.h"

int lua_bind_dispatch(lua_State *state)
{
	lua_CFunction function = lua_tocfunction(state, lua_upvalueindex(1));
	KmodLuaRequirement requirement = (KmodLuaRequirement)lua_tointeger(state, lua_upvalueindex(2));
	int systemRunning = Genesis_Started || SegaCD_Started || _32X_Started;

	if (function == NULL)
		return luaL_error(state, "Invalid KMod Lua binding.");
	if (requirement == KMOD_LUA_REQUIREMENT_SYSTEM && !systemRunning)
		return luaL_error(state, "This function requires an emulated system to be running.");
	if (requirement == KMOD_LUA_REQUIREMENT_GENESIS && !Genesis_Started)
		return luaL_error(state, "This function requires Genesis to be running.");

	return function(state);
}

void lua_bind_register(lua_State *state)
{
	lua_bind_emu_register(state);
	lua_bind_kmod_register(state);
	lua_bind_memory_register(state);
	lua_bind_joypad_register(state);
	lua_bind_savestate_register(state);
	lua_bind_palette_register(state);
	lua_bind_tile_register(state);
	lua_bind_vdp_register(state);
}
