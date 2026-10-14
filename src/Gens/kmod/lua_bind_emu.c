#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../G_main.h"
#include "../Mem_M68k.h"
#include "../Rom.h"
#include "../vdp_io.h"

static int LuaBindFrameCount(lua_State *state)
{
	lua_pushinteger(state, (lua_Integer)Frame_Number);
	return 1;
}

static int LuaBindGetRomName(lua_State *state)
{
	if (Game == NULL)
		lua_pushnil(state);
	else
		lua_pushstring(state, Game->Rom_Name_W);
	return 1;
}

static int LuaBindGetRomHeader(lua_State *state)
{
	if (Game == NULL)
	{
		lua_pushnil(state);
		return 1;
	}

	lua_createtable(state, 0, 10);
	lua_pushstring(state, Game->Console_Name);
	lua_setfield(state, -2, "console");
	lua_pushstring(state, Game->Copyright);
	lua_setfield(state, -2, "copyright");
	lua_pushstring(state, Game->Rom_Name_W);
	lua_setfield(state, -2, "name");
	lua_pushstring(state, Game->Type);
	lua_setfield(state, -2, "type");
	lua_pushstring(state, Game->Version);
	lua_setfield(state, -2, "version");
	lua_pushinteger(state, Game->Checksum);
	lua_setfield(state, -2, "checksum");
	lua_pushstring(state, Game->IO_Support);
	lua_setfield(state, -2, "io_support");
	lua_pushstring(state, Game->Description);
	lua_setfield(state, -2, "description");
	lua_pushstring(state, Game->Countries);
	lua_setfield(state, -2, "countries");
	return 1;
}

static int LuaBindGetCountry(lua_State *state)
{
	static const char *countries[] = {"usa_ntsc", "japan_ntsc", "europe_pal", "asia_pal"};
	if (Country < 0 || Country >= (int)(sizeof(countries) / sizeof(countries[0])))
		lua_pushnil(state);
	else
		lua_pushstring(state, countries[Country]);
	return 1;
}

static int LuaBindGetFrameSkip(lua_State *state)
{
	lua_pushinteger(state, Frame_Skip);
	return 1;
}

static int LuaBindGetVisibleLines(lua_State *state)
{
	if (!Genesis_Started)
		lua_pushnil(state);
	else
		lua_pushinteger(state, VDP_Num_Vis_Lines);
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

	lua_createtable(state, 0, 10);
	apiIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 9);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "framecount", LuaBindFrameCount, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "ispaused", LuaBindIsPaused, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getromsize", LuaBindGetRomSize, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getromname", LuaBindGetRomName, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getromheader", LuaBindGetRomHeader, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getcountry", LuaBindGetCountry, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getframeskip", LuaBindGetFrameSkip, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getvisiblelines", LuaBindGetVisibleLines, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getsystems", LuaBindGetSystems, "()");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, apiIndex, "signatures");
	lua_pop(state, 1);
	lua_pushvalue(state, apiIndex);
	lua_setglobal(state, "gens");
	lua_setglobal(state, "emu");
}
