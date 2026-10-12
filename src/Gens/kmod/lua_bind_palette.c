#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../vdp_io.h"
#include "vdp.h"

#define LUA_PALETTE_SIZE 16
#define LUA_PALETTE_METATABLE "kmod.palette"

typedef struct LuaPalette
{
	unsigned short colors[LUA_PALETTE_SIZE];
} LuaPalette;

static LuaPalette *LuaPaletteCheck(lua_State *state, int index)
{
	return (LuaPalette *)luaL_checkudata(state, index, LUA_PALETTE_METATABLE);
}

static int LuaPaletteCheckIndex(lua_State *state, int index)
{
	lua_Integer colorIndex = luaL_checkinteger(state, index);
	if (colorIndex < 1 || colorIndex > LUA_PALETTE_SIZE)
	{
		luaL_argerror(state, index, "color index must be between 1 and 16");
		return 0;
	}
	return (int)colorIndex - 1;
}

static unsigned int LuaPaletteCheckComponent(lua_State *state, int tableIndex, const char *name, unsigned int defaultValue)
{
	lua_Integer component;
	lua_getfield(state, tableIndex, name);
	if (lua_isnil(state, -1))
	{
		lua_pop(state, 1);
		return defaultValue;
	}
	component = luaL_checkinteger(state, -1);
	lua_pop(state, 1);
	if (component < 0 || component > 255)
	{
		luaL_error(state, "color component '%s' must be between 0 and 255", name);
		return 0;
	}
	return (unsigned int)component;
}

static int LuaBindPaletteCreate(lua_State *state)
{
	LuaPalette *palette = (LuaPalette *)lua_newuserdata(state, sizeof(LuaPalette));
	int i;
	for (i = 0; i < LUA_PALETTE_SIZE; i++)
		palette->colors[i] = 0xFFFF;
	luaL_setmetatable(state, LUA_PALETTE_METATABLE);
	return 1;
}

static int LuaBindPaletteGetColor(lua_State *state)
{
	LuaPalette *palette = LuaPaletteCheck(state, 1);
	int index = LuaPaletteCheckIndex(state, 2);
	unsigned short color = palette->colors[index];
	if (color == 0xFFFF)
	{
		lua_pushnil(state);
		return 1;
	}

	lua_createtable(state, 0, 4);
	lua_pushinteger(state, (color & 0x000E) << 4);
	lua_setfield(state, -2, "r");
	lua_pushinteger(state, color & 0x00E0);
	lua_setfield(state, -2, "g");
	lua_pushinteger(state, (color & 0x0E00) >> 4);
	lua_setfield(state, -2, "b");
	lua_pushinteger(state, 255);
	lua_setfield(state, -2, "a");
	return 1;
}

static int LuaBindPaletteSetColor(lua_State *state)
{
	LuaPalette *palette = LuaPaletteCheck(state, 1);
	int index = LuaPaletteCheckIndex(state, 2);
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int alpha;

	if (lua_isnoneornil(state, 3))
	{
		palette->colors[index] = 0xFFFF;
		return 0;
	}
	luaL_checktype(state, 3, LUA_TTABLE);
	red = LuaPaletteCheckComponent(state, 3, "r", 0);
	green = LuaPaletteCheckComponent(state, 3, "g", 0);
	blue = LuaPaletteCheckComponent(state, 3, "b", 0);
	alpha = LuaPaletteCheckComponent(state, 3, "a", 255);
	if (alpha == 0)
		palette->colors[index] = 0xFFFF;
	else
		palette->colors[index] = (unsigned short)(((red >> 5) << 1) | ((green >> 5) << 5) | ((blue >> 5) << 9));
	return 0;
}

static int LuaBindPaletteGetRaw(lua_State *state)
{
	LuaPalette *palette = LuaPaletteCheck(state, 1);
	int i;

	lua_createtable(state, LUA_PALETTE_SIZE * 2, 0);
	for (i = 0; i < LUA_PALETTE_SIZE; i++)
	{
		unsigned short color = palette->colors[i];
		if ((color >> 8) == 0xFF)
			lua_pushnil(state);
		else
			lua_pushinteger(state, color >> 8);
		lua_rawseti(state, -2, i * 2 + 1);
		if ((color & 0xFF) == 0xFF)
			lua_pushnil(state);
		else
			lua_pushinteger(state, color & 0xFF);
		lua_rawseti(state, -2, i * 2 + 2);
	}
	return 1;
}

static int LuaBindPaletteReadCramColor(lua_State *state)
{
	lua_Integer paletteIndex = luaL_checkinteger(state, 1);
	lua_Integer colorIndex = luaL_checkinteger(state, 2);
	COLORREF color;

	if (paletteIndex < 0 || paletteIndex > 3)
	{
		luaL_argerror(state, 1, "CRAM palette index must be between 0 and 3");
		return 0;
	}
	if (colorIndex < 0 || colorIndex > 15)
	{
		luaL_argerror(state, 2, "CRAM color index must be between 0 and 15");
		return 0;
	}
	color = vdpdebug_getColor((unsigned int)paletteIndex, (unsigned int)colorIndex);
	lua_createtable(state, 0, 4);
	lua_pushinteger(state, GetRValue(color));
	lua_setfield(state, -2, "r");
	lua_pushinteger(state, GetGValue(color));
	lua_setfield(state, -2, "g");
	lua_pushinteger(state, GetBValue(color));
	lua_setfield(state, -2, "b");
	lua_pushinteger(state, 255);
	lua_setfield(state, -2, "a");
	return 1;
}

void lua_bind_palette_register(lua_State *state)
{
	int paletteIndex;
	int signaturesIndex;
	int metatableIndex;

	luaL_newmetatable(state, LUA_PALETTE_METATABLE);
	metatableIndex = lua_absindex(state, -1);
	lua_pushvalue(state, metatableIndex);
	lua_setfield(state, metatableIndex, "__index");
	lua_pop(state, 1);

	lua_createtable(state, 0, 8);
	paletteIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 8);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "new", LuaBindPaletteCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "create", LuaBindPaletteCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "getcolor", LuaBindPaletteGetColor, "(palette, index)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "readcolor", LuaBindPaletteGetColor, "(palette, index)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "setcolor", LuaBindPaletteSetColor, "(palette, index, color)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "writecolor", LuaBindPaletteSetColor, "(palette, index, color)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "rawdata", LuaBindPaletteGetRaw, "(palette)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, signaturesIndex, "readcramcolor", LuaBindPaletteReadCramColor, "(palette, index)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, paletteIndex, "signatures");
	lua_pop(state, 1);
	lua_pushvalue(state, paletteIndex);
	lua_setglobal(state, "pal");
	lua_setglobal(state, "palette");
}
