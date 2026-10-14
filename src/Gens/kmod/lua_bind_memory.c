#include "lauxlib.h"
#include <string.h>

#include "lua_bind.h"
#include "../gens.h"
#include "../Mem_M68k.h"
#include "../Mem_S68k.h"
#include "../Mem_SH2.h"
#include "../Mem_Z80.h"
#include "../io.h"
#include "../vdp_io.h"
#include "../Star_68k.h"

#define LUA_MEMORY_MAX_RANGE (64 * 1024)

typedef struct LuaMemoryRegion
{
	lua_Integer start;
	lua_Integer size;
	unsigned char *data;
	int byteSwapped;
	int writable;
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
	candidate.writable = TRUE;
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
	candidate.writable = TRUE;
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
	candidate.writable = TRUE;
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
		candidate.writable = TRUE;
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
		candidate.writable = TRUE;
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
		candidate.writable = TRUE;
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

static int LuaMemoryWriteByteAt(lua_Integer address, unsigned char value)
{
	LuaMemoryRegion region;
	unsigned int offset;

	if (!LuaMemoryGetRegion(address, 1, &region) || !region.writable)
		return FALSE;
	offset = (unsigned int)(address - region.start);
	if (region.byteSwapped)
		offset ^= 1;
	region.data[offset] = value;
	return TRUE;
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

static void *LuaMemoryGetRegister(const char *name, int *size)
{
	struct S68000CONTEXT *context = &main68k_context;
	const char *registerName = name;
	int index;

	if (_strnicmp(registerName, "main.", 5) == 0 || _strnicmp(registerName, "m68k.", 5) == 0)
		registerName += 5;
	else if (_strnicmp(registerName, "sub.", 4) == 0 || _strnicmp(registerName, "s68k.", 5) == 0)
	{
		registerName += _strnicmp(registerName, "sub.", 4) == 0 ? 4 : 5;
		context = &sub68k_context;
	}

	if ((registerName[0] == 'a' || registerName[0] == 'A' || registerName[0] == 'd' || registerName[0] == 'D') &&
		registerName[1] >= '0' && registerName[1] <= '7' && registerName[2] == '\0')
	{
		index = registerName[1] - '0';
		*size = 4;
		return (registerName[0] == 'a' || registerName[0] == 'A') ? (void *)&context->areg[index] : (void *)&context->dreg[index];
	}
	if (_stricmp(registerName, "pc") == 0)
	{
		*size = 4;
		return &context->pc;
	}
	if (_stricmp(registerName, "sr") == 0)
	{
		*size = 2;
		return &context->sr;
	}
	return NULL;
}

static int LuaBindMemoryGetRegister(lua_State *state)
{
	const char *name = luaL_checkstring(state, 1);
	void *reg;
	int size;

	reg = LuaMemoryGetRegister(name, &size);
	if (reg == NULL)
	{
		lua_pushnil(state);
		return 1;
	}
	if (size == 2)
		lua_pushinteger(state, *(unsigned short *)reg);
	else
		lua_pushinteger(state, *(unsigned int *)reg);
	return 1;
}

static int LuaBindMemorySetRegister(lua_State *state)
{
	const char *name = luaL_checkstring(state, 1);
	lua_Integer value = luaL_checkinteger(state, 2);
	void *reg;
	int size;

	reg = LuaMemoryGetRegister(name, &size);
	if (reg != NULL)
	{
		if (size == 2)
			*(unsigned short *)reg = (unsigned short)value;
		else
			*(unsigned int *)reg = (unsigned int)value;
	}
	return 0;
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

static int LuaMemoryWriteValue(lua_State *state, lua_Integer address, lua_Integer value, int size)
{
	LuaMemoryRegion region;
	unsigned int mask = size == 4 ? 0xFFFFFFFFU : ((1U << (size * 8)) - 1);
	int offset;

	if (!LuaMemoryGetRegion(address, size, &region))
		return luaL_argerror(state, 1, "address range is outside an available memory region");
	if (value < 0 || (unsigned long long)value > mask)
		return luaL_argerror(state, 2, "value does not fit the requested unsigned width");
	for (offset = 0; offset < size; offset++)
		LuaMemoryWriteByteAt(address + offset, (unsigned char)(((unsigned int)value >> ((size - offset - 1) * 8)) & 0xFF));
	return 0;
}

static int LuaBindMemoryIsValid(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer size = luaL_optinteger(state, 2, 1);
	lua_pushboolean(state, LuaMemoryIsValidRange(address, size));
	return 1;
}

static int LuaBindMemoryWriteByte(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer value = luaL_checkinteger(state, 2);
	return LuaMemoryWriteValue(state, address, value, 1);
}

static int LuaBindMemoryWriteWord(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer value = luaL_checkinteger(state, 2);
	return LuaMemoryWriteValue(state, address, value, 2);
}

static int LuaBindMemoryWriteDWord(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer value = luaL_checkinteger(state, 2);
	return LuaMemoryWriteValue(state, address, value, 4);
}

static int LuaBindMemoryWriteByteRange(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	int length;
	int i;
	LuaMemoryRegion region;

	luaL_checktype(state, 2, LUA_TTABLE);
	length = (int)lua_rawlen(state, 2);
	if (length > LUA_MEMORY_MAX_RANGE)
		return luaL_argerror(state, 2, "byte range exceeds the 64 KiB limit");
	if (!LuaMemoryGetRegion(address, length, &region))
		return luaL_argerror(state, 1, "address range is outside an available memory region");
	for (i = 1; i <= length; i++)
	{
		lua_Integer value;
		lua_rawgeti(state, 2, i);
		value = luaL_checkinteger(state, -1);
		lua_pop(state, 1);
		if (value < 0 || value > 0xFF)
			return luaL_argerror(state, 2, "byte range values must be between 0 and 255");
	}
	for (i = 1; i <= length; i++)
	{
		lua_rawgeti(state, 2, i);
		LuaMemoryWriteByteAt(address + i - 1, (unsigned char)lua_tointeger(state, -1));
		lua_pop(state, 1);
	}
	lua_pushinteger(state, length);
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

	lua_createtable(state, 0, 24);
	memoryIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 23);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "isvalid", LuaBindMemoryIsValid, "(address[, size])");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbyte", LuaBindMemoryReadByte, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbytesigned", LuaBindMemoryReadByteSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readword", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readwordsigned", LuaBindMemoryReadWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readdword", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readdwordsigned", LuaBindMemoryReadDWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbyterange", LuaBindMemoryReadByteRange, "(address, length)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writebyte", LuaBindMemoryWriteByte, "(address, value)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writeword", LuaBindMemoryWriteWord, "(address, value)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writedword", LuaBindMemoryWriteDWord, "(address, value)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writebyterange", LuaBindMemoryWriteByteRange, "(address, bytes)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readbyteunsigned", LuaBindMemoryReadByte, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readwordunsigned", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readdwordunsigned", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readshort", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readshortunsigned", LuaBindMemoryReadWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readshortsigned", LuaBindMemoryReadWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readlong", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readlongunsigned", LuaBindMemoryReadDWord, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "readlongsigned", LuaBindMemoryReadDWordSigned, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writeshort", LuaBindMemoryWriteWord, "(address, value)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "writelong", LuaBindMemoryWriteDWord, "(address, value)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "getregister", LuaBindMemoryGetRegister, "(registerName)");
	KMOD_LUA_BIND_FUNCTION(state, memoryIndex, signaturesIndex, "setregister", LuaBindMemorySetRegister, "(registerName, value)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, memoryIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "memory");
}
