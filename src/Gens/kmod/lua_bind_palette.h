#ifndef KMOD_LUA_BIND_PALETTE_H
#define KMOD_LUA_BIND_PALETTE_H

#include "lua.h"

#define LUA_PALETTE_SIZE 16

typedef struct LuaPalette
{
	unsigned short colors[LUA_PALETTE_SIZE];
} LuaPalette;

#ifdef __cplusplus
extern "C" {
#endif

void LuaPalettePush(lua_State *state, const unsigned char *colors);
LuaPalette *LuaPaletteCheck(lua_State *state, int index);

#ifdef __cplusplus
}
#endif

#endif