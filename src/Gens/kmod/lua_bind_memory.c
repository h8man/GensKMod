#include "lauxlib.h"

#include "lua_bind.h"
#include "../gens.h"
#include "../Mem_M68k.h"
#include "../Mem_S68k.h"
#include "../Mem_SH2.h"
#include "../Mem_Z80.h"
#include "../io.h"
#include "../vdp_io.h"

#define LUA_MEMORY_MAX_RANGE (64 * 1024)

typedef struct LuaMemoryRegion
{
	lua_Integer start;
	lua_Integer size;
	unsigned char *data;
	int byteSwapped;
} LuaMemoryRegion;

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

void lua_bind_memory_register(lua_State *state)
{
	int memoryIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 9);
	memoryIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 8);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "isvalid", LuaBindMemoryIsValid, "(address[, size])");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbyte", LuaBindMemoryReadByte, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbytesigned", LuaBindMemoryReadByteSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readword", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readwordsigned", LuaBindMemoryReadWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readdword", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readdwordsigned", LuaBindMemoryReadDWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbyterange", LuaBindMemoryReadByteRange, "(address, length)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, memoryIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "memory");
}
