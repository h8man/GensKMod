#include "lauxlib.h"

#include "lua_bind.h"
#include "lua_script.h"
#include "vdp.h"
#include "../gens.h"
#include "../G_main.h"
#include "../Mem_M68k.h"
#include "../Mem_S68k.h"
#include "../Mem_SH2.h"
#include "../Mem_Z80.h"
#include "../io.h"
#include "../vdp_io.h"

#define LUA_MEMORY_MAX_RANGE (64 * 1024)
#define LUA_PALETTE_SIZE 16
#define LUA_PALETTE_METATABLE "kmod.palette"

typedef struct LuaMemoryRegion
{
	lua_Integer start;
	lua_Integer size;
	unsigned char *data;
	int byteSwapped;
} LuaMemoryRegion;

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

typedef struct LuaPalette
{
	unsigned short colors[LUA_PALETTE_SIZE];
} LuaPalette;

#define KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, name, function, signature) \
	do { \
		lua_pushcfunction(state, function); \
		lua_setfield(state, apiIndex, name); \
		lua_pushstring(state, signature); \
		lua_setfield(state, signaturesIndex, name); \
	} while (0)

static int LuaMemoryGetRegion(lua_Integer address, lua_Integer size, LuaMemoryRegion *region)
{
	LuaMemoryRegion candidate;

	if (address < 0 || size < 0)
		return FALSE;

	candidate.start = 0;
	candidate.size = (lua_Integer)Rom_Size;
	candidate.data = Rom_Data;
	candidate.byteSwapped = TRUE;
	if (address >= candidate.start && address <= candidate.start + candidate.size &&
		size <= candidate.size - (address - candidate.start))
	{
		*region = candidate;
		return TRUE;
	}

	candidate.start = 0xFF0000;
	candidate.size = sizeof(Ram_68k);
	candidate.data = Ram_68k;
	candidate.byteSwapped = TRUE;
	if (address >= candidate.start && address <= candidate.start + candidate.size &&
		size <= candidate.size - (address - candidate.start))
	{
		*region = candidate;
		return TRUE;
	}

	candidate.start = 0xA00000;
	candidate.size = sizeof(Ram_Z80);
	candidate.data = Ram_Z80;
	candidate.byteSwapped = TRUE;
	if (address >= candidate.start && address <= candidate.start + candidate.size &&
		size <= candidate.size - (address - candidate.start))
	{
		*region = candidate;
		return TRUE;
	}

	if (SegaCD_Started)
	{
		candidate.start = 0x020000;
		candidate.size = sizeof(Ram_Prg);
		candidate.data = Ram_Prg;
		candidate.byteSwapped = TRUE;
		if (address >= candidate.start && address <= candidate.start + candidate.size &&
			size <= candidate.size - (address - candidate.start))
		{
			*region = candidate;
			return TRUE;
		}

		candidate.start = 0x200000;
		candidate.size = (Ram_Word_State & 2) ? sizeof(Ram_Word_1M) : sizeof(Ram_Word_2M);
		candidate.data = (Ram_Word_State & 2) ? Ram_Word_1M : Ram_Word_2M;
		candidate.byteSwapped = TRUE;
		if (address >= candidate.start && address <= candidate.start + candidate.size &&
			size <= candidate.size - (address - candidate.start))
		{
			*region = candidate;
			return TRUE;
		}
	}

	if (_32X_Started)
	{
		candidate.start = 0x06000000;
		candidate.size = sizeof(_32X_Ram);
		candidate.data = _32X_Ram;
		candidate.byteSwapped = FALSE;
		if (address >= candidate.start && address <= candidate.start + candidate.size &&
			size <= candidate.size - (address - candidate.start))
		{
			*region = candidate;
			return TRUE;
		}
	}

	return FALSE;
}

static int LuaMemoryIsValidRange(lua_Integer address, lua_Integer size)
{
	LuaMemoryRegion region;
	return LuaMemoryGetRegion(address, size, &region);
}

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

static unsigned char LuaMemoryReadByteAt(lua_Integer address)
{
	LuaMemoryRegion region;
	unsigned int offset;

	LuaMemoryGetRegion(address, 1, &region);
	offset = (unsigned int)(address - region.start);
	if (region.byteSwapped)
		offset ^= 1;
	return region.data[offset];
}

static unsigned int LuaMemoryReadValue(lua_State *state, lua_Integer address, int size)
{
	unsigned int value = 0;
	int offset;

	if (!LuaMemoryIsValidRange(address, size))
	{
		luaL_argerror(state, 1, "address range is outside an available memory region");
		return 0;
	}

	for (offset = 0; offset < size; offset++)
		value = (value << 8) | LuaMemoryReadByteAt(address + offset);
	return value;
}

static lua_Integer LuaMemorySignExtend(unsigned int value, int size)
{
	if (size == 1 && value >= 0x80)
		return (lua_Integer)value - 0x100;
	if (size == 2 && value >= 0x8000)
		return (lua_Integer)value - 0x10000;
	if (size == 4 && value >= 0x80000000U)
		return (lua_Integer)value - 0x100000000LL;
	return (lua_Integer)value;
}

static int LuaBindFrameCount(lua_State *state)
{
	lua_pushinteger(state, (lua_Integer)Frame_Number);
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

static int LuaBindMemoryIsValid(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer size = luaL_optinteger(state, 2, 1);

	lua_pushboolean(state, LuaMemoryIsValidRange(address, size));
	return 1;
}

static int LuaBindMemoryReadByte(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_pushinteger(state, LuaMemoryReadValue(state, address, 1));
	return 1;
}

static int LuaBindMemoryReadByteSigned(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	unsigned int value = LuaMemoryReadValue(state, address, 1);
	lua_pushinteger(state, LuaMemorySignExtend(value, 1));
	return 1;
}

static int LuaBindMemoryReadWord(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_pushinteger(state, LuaMemoryReadValue(state, address, 2));
	return 1;
}

static int LuaBindMemoryReadWordSigned(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	unsigned int value = LuaMemoryReadValue(state, address, 2);
	lua_pushinteger(state, LuaMemorySignExtend(value, 2));
	return 1;
}

static int LuaBindMemoryReadDWord(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_pushinteger(state, LuaMemoryReadValue(state, address, 4));
	return 1;
}

static int LuaBindMemoryReadDWordSigned(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	unsigned int value = LuaMemoryReadValue(state, address, 4);
	lua_pushinteger(state, LuaMemorySignExtend(value, 4));
	return 1;
}

static int LuaBindMemoryReadByteRange(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer length = luaL_checkinteger(state, 2);
	lua_Integer offset;

	if (length < 0)
	{
		luaL_argerror(state, 2, "length must not be negative");
		return 0;
	}
	if (length > LUA_MEMORY_MAX_RANGE)
	{
		luaL_argerror(state, 2, "length exceeds the 64 KiB limit");
		return 0;
	}
	if (!LuaMemoryIsValidRange(address, length))
	{
		luaL_argerror(state, 1, "address range is outside an available memory region");
		return 0;
	}

	lua_createtable(state, (int)length, 0);
	for (offset = 0; offset < length; offset++)
	{
		lua_pushinteger(state, LuaMemoryReadByteAt(address + offset));
		lua_rawseti(state, -2, offset + 1);
	}
	return 1;
}

static int LuaBindLog(lua_State *state)
{
	const char *message = luaL_checkstring(state, 1);
	kmod_lua_output("KMOD", message);
	return 0;
}

void lua_bind_register(lua_State *state)
{
	int apiIndex;
	int signaturesIndex;
	int kmodIndex;
	int kmodSignaturesIndex;
	int memoryIndex;
	int memorySignaturesIndex;
	int joypadIndex;
	int joypadSignaturesIndex;
	int paletteIndex;
	int paletteSignaturesIndex;
	int paletteMetaIndex;

	lua_createtable(state, 0, 5);
	apiIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 4);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "framecount", LuaBindFrameCount, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "ispaused", LuaBindIsPaused, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getromsize", LuaBindGetRomSize, "()");
	KMOD_LUA_BIND_FUNCTION(state, apiIndex, signaturesIndex, "getsystems", LuaBindGetSystems, "()");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, apiIndex, "signatures");
	lua_pop(state, 1);
	lua_pushvalue(state, apiIndex);
	lua_setglobal(state, "gens");
	lua_setglobal(state, "emu");

	lua_createtable(state, 0, 2);
	kmodIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 1);
	kmodSignaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, kmodIndex, kmodSignaturesIndex, "log", LuaBindLog, "(message)");
	lua_pushvalue(state, kmodSignaturesIndex);
	lua_setfield(state, kmodIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "kmod");

	lua_createtable(state, 0, 9);
	memoryIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 8);
	memorySignaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "isvalid", LuaBindMemoryIsValid, "(address[, size])");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readbyte", LuaBindMemoryReadByte, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readbytesigned", LuaBindMemoryReadByteSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readword", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readwordsigned", LuaBindMemoryReadWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readdword", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readdwordsigned", LuaBindMemoryReadDWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, memorySignaturesIndex, "readbyterange", LuaBindMemoryReadByteRange, "(address, length)");
	lua_pushvalue(state, memorySignaturesIndex);
	lua_setfield(state, memoryIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "memory");

	lua_createtable(state, 0, 2);
	joypadIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 1);
	joypadSignaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, joypadIndex, joypadSignaturesIndex, "get", LuaBindJoypadGet, "(player)");
	lua_pushvalue(state, joypadSignaturesIndex);
	lua_setfield(state, joypadIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "joypad");

	luaL_newmetatable(state, LUA_PALETTE_METATABLE);
	paletteMetaIndex = lua_absindex(state, -1);
	lua_pushvalue(state, paletteMetaIndex);
	lua_setfield(state, paletteMetaIndex, "__index");
	lua_pop(state, 1);

	lua_createtable(state, 0, 8);
	paletteIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 8);
	paletteSignaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "new", LuaBindPaletteCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "create", LuaBindPaletteCreate, "()");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "getcolor", LuaBindPaletteGetColor, "(palette, index)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "readcolor", LuaBindPaletteGetColor, "(palette, index)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "setcolor", LuaBindPaletteSetColor, "(palette, index, color)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "writecolor", LuaBindPaletteSetColor, "(palette, index, color)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "rawdata", LuaBindPaletteGetRaw, "(palette)");
	KMOD_LUA_BIND_FUNCTION(state, paletteIndex, paletteSignaturesIndex, "readcramcolor", LuaBindPaletteReadCramColor, "(palette, index)");
	lua_pushvalue(state, paletteSignaturesIndex);
	lua_setfield(state, paletteIndex, "signatures");
	lua_pop(state, 1);
	lua_pushvalue(state, paletteIndex);
	lua_setglobal(state, "pal");
	lua_setglobal(state, "palette");
}
