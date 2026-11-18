#include "lauxlib.h"
#include <windows.h>
#include <limits.h>
#include <string.h>

#include "lua_bind.h"
#include "../gens.h"
#include "common.h"
#include "lua_module.h"
#include "lua_script.h"

typedef struct KmodWindowName
{
	const char *name;
	UCHAR mode;
} KmodWindowName;

static const KmodWindowName kmodWindowNames[] =
{
	{ "message", DMODE_MSG },
	{ "watchers", DMODE_WATCHERS },
	{ "layers", DMODE_LAYERS },
	{ "plane_explorer", DMODE_PLANEEXPLORER },
	{ "plane_composite", DMODE_PLANECOMPOSITE },
	{ "lua_script", DMODE_LUASCRIPT },
	{ "genesis_68k", DMODE_68K },
	{ "genesis_z80", DMODE_Z80 },
	{ "genesis_vdp", DMODE_VDP },
	{ "genesis_vdp_registers", DMODE_VDP_REG },
	{ "genesis_sprites", DMODE_SPRITES },
	{ "genesis_ym2612", DMODE_YM2612 },
	{ "genesis_psg", DMODE_PSG },
	{ "sega_cd_68k", DMODE_CD_68K },
	{ "sega_cd_cdc", DMODE_CD_CDC },
	{ "sega_cd_gfx", DMODE_CD_GFX },
	{ "sega_cd_registers", DMODE_CD_REG },
	{ "32x_main_sh2", DMODE_32_MSH2 },
	{ "32x_sub_sh2", DMODE_32_SSH2 },
	{ "32x_vdp", DMODE_32_VDP },
	{ "32x_registers", DMODE_32_REG }
};

static int LuaBindLog(lua_State *state)
{
	const char *message = luaL_checkstring(state, 1);
	kmod_lua_output("KMOD", message);
	return 0;
}

static int LuaBindClearLog(lua_State *state)
{
	(void)state;
	kmod_lua_clear_output();
	return 0;
}

static int LuaBindShouldStop(lua_State *state)
{
	lua_pushboolean(state, lua_module_should_stop());
	return 1;
}

static UCHAR LuaBindWindowMode(lua_State *state, int index)
{
	size_t nameLength;
	const char *name = luaL_checklstring(state, index, &nameLength);
	size_t i;

	for (i = 0; i < sizeof(kmodWindowNames) / sizeof(kmodWindowNames[0]); i++)
	{
		if (nameLength == strlen(kmodWindowNames[i].name) &&
			memcmp(name, kmodWindowNames[i].name, nameLength) == 0)
			return kmodWindowNames[i].mode;
	}

	luaL_argerror(state, index, "unknown KMod window name");
	return 0;
}

static int LuaBindSetWindowVisible(lua_State *state)
{
	UCHAR mode = LuaBindWindowMode(state, 1);
	BOOL visible;

	luaL_checktype(state, 2, LUA_TBOOLEAN);
	visible = lua_toboolean(state, 2);
	lua_pushboolean(state, SetKModWindowVisible(mode, visible));
	return 1;
}

static int LuaBindSetWindowPosition(lua_State *state)
{
	UCHAR mode = LuaBindWindowMode(state, 1);
	lua_Integer x = luaL_checkinteger(state, 2);
	lua_Integer y = luaL_checkinteger(state, 3);

	if (x < INT_MIN || x > INT_MAX)
		return luaL_argerror(state, 2, "x coordinate is outside the supported range");
	if (y < INT_MIN || y > INT_MAX)
		return luaL_argerror(state, 3, "y coordinate is outside the supported range");

	lua_pushboolean(state, SetKModWindowPosition(mode, (int)x, (int)y));
	return 1;
}

void lua_bind_kmod_register(lua_State *state)
{
	int kmodIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 6);
	kmodIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 5);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "log", LuaBindLog, "(message)");
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "clearlog", LuaBindClearLog, "()");
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "should_stop", LuaBindShouldStop, "() -> boolean");
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "set_window_visible", LuaBindSetWindowVisible, "(name, visible) -> boolean");
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, signaturesIndex, "set_window_position", LuaBindSetWindowPosition, "(name, x, y) -> boolean");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, kmodIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "kmod");
}
