#include "lua_bind.h"

void lua_bind_register(lua_State *state)
{
	lua_bind_emu_register(state);
	lua_bind_kmod_register(state);
	lua_bind_memory_register(state);
	lua_bind_joypad_register(state);
	lua_bind_palette_register(state);
	lua_bind_tile_register(state);
}
