#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../io.h"

typedef struct LuaJoypadState
{
	unsigned int *up;
	unsigned int *down;
	unsigned int *left;
	unsigned int *right;
	unsigned int *a;
	unsigned int *b;
	unsigned int *c;
	unsigned int *x;
	unsigned int *y;
	unsigned int *z;
	unsigned int *start;
	unsigned int *mode;
} LuaJoypadState;

static int LuaJoypadGetState(lua_Integer player, LuaJoypadState *pad)
{
	static LuaJoypadState pads[] = {
		{&Controller_1_Up, &Controller_1_Down, &Controller_1_Left, &Controller_1_Right, &Controller_1_A, &Controller_1_B, &Controller_1_C, &Controller_1_X, &Controller_1_Y, &Controller_1_Z, &Controller_1_Start, &Controller_1_Mode},
		{&Controller_2_Up, &Controller_2_Down, &Controller_2_Left, &Controller_2_Right, &Controller_2_A, &Controller_2_B, &Controller_2_C, &Controller_2_X, &Controller_2_Y, &Controller_2_Z, &Controller_2_Start, &Controller_2_Mode},
		{&Controller_1B_Up, &Controller_1B_Down, &Controller_1B_Left, &Controller_1B_Right, &Controller_1B_A, &Controller_1B_B, &Controller_1B_C, &Controller_1B_X, &Controller_1B_Y, &Controller_1B_Z, &Controller_1B_Start, &Controller_1B_Mode},
		{&Controller_1C_Up, &Controller_1C_Down, &Controller_1C_Left, &Controller_1C_Right, &Controller_1C_A, &Controller_1C_B, &Controller_1C_C, &Controller_1C_X, &Controller_1C_Y, &Controller_1C_Z, &Controller_1C_Start, &Controller_1C_Mode},
		{&Controller_1D_Up, &Controller_1D_Down, &Controller_1D_Left, &Controller_1D_Right, &Controller_1D_A, &Controller_1D_B, &Controller_1D_C, &Controller_1D_X, &Controller_1D_Y, &Controller_1D_Z, &Controller_1D_Start, &Controller_1D_Mode},
		{&Controller_2B_Up, &Controller_2B_Down, &Controller_2B_Left, &Controller_2B_Right, &Controller_2B_A, &Controller_2B_B, &Controller_2B_C, &Controller_2B_X, &Controller_2B_Y, &Controller_2B_Z, &Controller_2B_Start, &Controller_2B_Mode},
		{&Controller_2C_Up, &Controller_2C_Down, &Controller_2C_Left, &Controller_2C_Right, &Controller_2C_A, &Controller_2C_B, &Controller_2C_C, &Controller_2C_X, &Controller_2C_Y, &Controller_2C_Z, &Controller_2C_Start, &Controller_2C_Mode},
		{&Controller_2D_Up, &Controller_2D_Down, &Controller_2D_Left, &Controller_2D_Right, &Controller_2D_A, &Controller_2D_B, &Controller_2D_C, &Controller_2D_X, &Controller_2D_Y, &Controller_2D_Z, &Controller_2D_Start, &Controller_2D_Mode}
	};

	if (player < 1 || player > (lua_Integer)(sizeof(pads) / sizeof(pads[0])))
		return FALSE;
	if (player >= 3 && player <= 5 && !(Controller_1_Type & 0x10))
		return FALSE;
	if (player >= 6 && !(Controller_2_Type & 0x10))
		return FALSE;

	*pad = pads[player - 1];
	return TRUE;
}

static void LuaJoypadPushButton(lua_State *state, const char *name, unsigned int *button)
{
	lua_pushboolean(state, *button == 0);
	lua_setfield(state, -2, name);
}

static int LuaBindJoypadGet(lua_State *state)
{
	lua_Integer player = luaL_checkinteger(state, 1);
	LuaJoypadState pad;

	if (!LuaJoypadGetState(player, &pad))
	{
		luaL_argerror(state, 1, "player is not available (use 1-2, or enable Team Player for expanded slots)");
		return 0;
	}

	lua_createtable(state, 0, 12);
	LuaJoypadPushButton(state, "up", pad.up);
	LuaJoypadPushButton(state, "down", pad.down);
	LuaJoypadPushButton(state, "left", pad.left);
	LuaJoypadPushButton(state, "right", pad.right);
	LuaJoypadPushButton(state, "a", pad.a);
	LuaJoypadPushButton(state, "b", pad.b);
	LuaJoypadPushButton(state, "c", pad.c);
	LuaJoypadPushButton(state, "x", pad.x);
	LuaJoypadPushButton(state, "y", pad.y);
	LuaJoypadPushButton(state, "z", pad.z);
	LuaJoypadPushButton(state, "start", pad.start);
	LuaJoypadPushButton(state, "mode", pad.mode);
	return 1;
}

void lua_bind_joypad_register(lua_State *state)
{
	int joypadIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 2);
	joypadIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 1);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, joypadIndex, signaturesIndex, "get", LuaBindJoypadGet, "(player)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, joypadIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "joypad");
}
