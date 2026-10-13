#ifndef KMOD_LUA_BIND_TILE_H
#define KMOD_LUA_BIND_TILE_H

#include "lua.h"

#define LUA_TILE_SIZE 32

typedef struct LuaTile
{
	unsigned char pixels[LUA_TILE_SIZE];
} LuaTile;

#ifdef __cplusplus
extern "C" {
#endif

void LuaTilePush(lua_State *state, const unsigned char *pixels);
LuaTile *LuaTileCheck(lua_State *state, int index);

#ifdef __cplusplus
}
#endif

#endif