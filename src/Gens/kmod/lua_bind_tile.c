#include "lauxlib.h"

#include "lua_bind.h"

#define LUA_TILE_SIZE 32
#define LUA_TILE_METATABLE "kmod.tile"

typedef struct LuaTile
{
	unsigned char pixels[LUA_TILE_SIZE];
} LuaTile;

static LuaTile *LuaTileCheck(lua_State *state, int index)
{
	return (LuaTile *)luaL_checkudata(state, index, LUA_TILE_METATABLE);
}

static int LuaTileCheckCoordinate(lua_State *state, int index)
{
	lua_Integer coordinate = luaL_checkinteger(state, index);
	if (coordinate < 1 || coordinate > 8)
	{
		luaL_argerror(state, index, "tile coordinate must be between 1 and 8");
		return 0;
	}
	return (int)coordinate - 1;
}

static int LuaBindTileCreate(lua_State *state)
{
	LuaTile *tile = (LuaTile *)lua_newuserdata(state, sizeof(LuaTile));
	int i;
	for (i = 0; i < LUA_TILE_SIZE; i++)
		tile->pixels[i] = 0;
	luaL_setmetatable(state, LUA_TILE_METATABLE);
	return 1;
}

static int LuaBindTileGetPixel(lua_State *state)
{
	LuaTile *tile = LuaTileCheck(state, 1);
	int x = LuaTileCheckCoordinate(state, 2);
	int y = LuaTileCheckCoordinate(state, 3);
	unsigned char packed = tile->pixels[y * 4 + x / 2];
	unsigned int color = (x % 2 == 0) ? (packed >> 4) : (packed & 0x0F);
	lua_pushinteger(state, color);
	return 1;
}

static int LuaBindTileSetPixel(lua_State *state)
{
	LuaTile *tile = LuaTileCheck(state, 1);
	int x = LuaTileCheckCoordinate(state, 2);
	int y = LuaTileCheckCoordinate(state, 3);
	lua_Integer color = lua_isnoneornil(state, 4) ? 0 : luaL_checkinteger(state, 4);
	int byteIndex = y * 4 + x / 2;

	if (color < 0 || color > 15)
	{
		luaL_argerror(state, 4, "pixel color must be between 0 and 15");
		return 0;
	}
	if (x % 2 == 0)
		tile->pixels[byteIndex] = (unsigned char)((tile->pixels[byteIndex] & 0x0F) | ((unsigned int)color << 4));
	else
		tile->pixels[byteIndex] = (unsigned char)((tile->pixels[byteIndex] & 0xF0) | (unsigned int)color);
	return 0;
}

static int LuaBindTileGetRaw(lua_State *state)
{
	LuaTile *tile = LuaTileCheck(state, 1);
	int i;
	lua_createtable(state, LUA_TILE_SIZE, 0);
	for (i = 0; i < LUA_TILE_SIZE; i++)
	{
		lua_pushinteger(state, tile->pixels[i]);
		lua_rawseti(state, -2, i + 1);
	}
	return 1;
}

void lua_bind_tile_register(lua_State *state)
{
	int tileIndex;
	int signaturesIndex;
	int metatableIndex;

	luaL_newmetatable(state, LUA_TILE_METATABLE);
	metatableIndex = lua_absindex(state, -1);
	lua_pushvalue(state, metatableIndex);
	lua_setfield(state, metatableIndex, "__index");
	lua_pop(state, 1);

	lua_createtable(state, 0, 7);
	tileIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 7);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "new", LuaBindTileCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "create", LuaBindTileCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "getpixel", LuaBindTileGetPixel, "(tile, x, y)");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "readpixel", LuaBindTileGetPixel, "(tile, x, y)");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "setpixel", LuaBindTileSetPixel, "(tile, x, y, color)");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "writepixel", LuaBindTileSetPixel, "(tile, x, y, color)");
	KMOD_LUA_BIND_FUNCTION(state, tileIndex, signaturesIndex, "rawdata", LuaBindTileGetRaw, "(tile)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, tileIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "tile");
}
