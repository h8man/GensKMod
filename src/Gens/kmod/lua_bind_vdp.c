#include "lauxlib.h"

#include "lua_bind.h"
#include "lua_bind_tile.h"
#include "lua_bind_palette.h"
#include "../vdp_io.h"
#include <string.h>

#define LUA_VDP_VRAM_SIZE sizeof(VRam)
#define LUA_VDP_CRAM_LINE_COUNT 4
#define LUA_VDP_CRAM_LINE_SIZE 32
#define LUA_VDP_VSRAM_SIZE sizeof(VSRam)

enum LuaVdpVramTable
{
	LUA_VDP_PATTERN_TABLE,
	LUA_VDP_OTHER_TABLE
};

static int LuaVdpIsAvailable(lua_State *state)
{
	if (!Genesis_Started)
	{
		luaL_error(state, "VDP is unavailable when Genesis is not running");
		return 0;
	}
	return 1;
}

static enum LuaVdpVramTable LuaVdpGetVramTable(unsigned int address)
{
	unsigned int tableAStart = (VDP_Reg.Pat_ScrA_Adr & 0x38) << 10;
	unsigned int tableBStart = (VDP_Reg.Pat_ScrB_Adr & 0x3) << 13;
	unsigned int tableWStart;
	unsigned int tableWSize;
	unsigned int tableSStart;
	unsigned int tableSSize = 640;
	unsigned int scrollStart = (VDP_Reg.H_Scr_Adr & 0x3F) << 10;
	unsigned int scrollSize;
	unsigned int scrollSizeReg = ((VDP_Reg.Scr_Size & 0x30) >> 2) | (VDP_Reg.Scr_Size & 0x3);
	unsigned int tableASize;
	unsigned int tableBSize;
	unsigned int sizeIndex;

	if (VDP_Reg.Set4 & 0x81)
	{
		tableWStart = (VDP_Reg.Pat_Win_Adr & 0x3C) << 10;
		tableWSize = 0x1000;
		tableSStart = (VDP_Reg.Spr_Att_Adr & 0x7E) << 9;
	}
	else
	{
		tableWStart = (VDP_Reg.Pat_Win_Adr & 0x3E) << 10;
		tableWSize = 0x800;
		tableSStart = (VDP_Reg.Spr_Att_Adr & 0x7F) << 9;
	}

	switch (scrollSizeReg & 0xF)
	{
	case 1:
	case 4:
		tableASize = 0x1000;
		tableBSize = 0x1000;
		break;
	case 3:
	case 5:
	case 12:
		tableASize = 0x2000;
		tableBSize = 0x2000;
		break;
	default:
		tableASize = 0x800;
		tableBSize = 0x800;
		break;
	}

	if ((VDP_Reg.Set3 & 0x3) == 0x3)
		scrollSize = 0x400;
	else if (VDP_Reg.Set3 & 0x3)
		scrollSize = 0x3F4;
	else
		scrollSize = 4;

	for (sizeIndex = 0; sizeIndex < 5; sizeIndex++)
	{
		unsigned int start;
		unsigned int size;
		switch (sizeIndex)
		{
		case 0: start = tableAStart; size = tableASize; break;
		case 1: start = tableBStart; size = tableBSize; break;
		case 2: start = tableWStart; size = tableWSize; break;
		case 3: start = tableSStart; size = tableSSize; break;
		default: start = scrollStart; size = scrollSize; break;
		}
		if (address >= start && address - start < size)
			return LUA_VDP_OTHER_TABLE;
	}
	return LUA_VDP_PATTERN_TABLE;
}

static int LuaVdpIsCellAddressValid(lua_Integer address)
{
	if (address < 0 || (unsigned long long)address + LUA_VDP_CRAM_LINE_SIZE > LUA_VDP_VRAM_SIZE)
		return 0;
	if (LuaVdpGetVramTable((unsigned int)address) != LUA_VDP_PATTERN_TABLE)
		return 0;
	return 1;
}

static int LuaBindVdpReadCell(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	lua_Integer requestedCount = luaL_optinteger(state, 2, 1);
	lua_Integer count;
	lua_Integer startAddress = address;
	lua_Integer i;
	int readCount = 0;

	LuaVdpIsAvailable(state);
	if (requestedCount < -2000)
		requestedCount = -2000;
	if (requestedCount > 2000)
		requestedCount = 2000;
	count = requestedCount < 0 ? -requestedCount : requestedCount;
	if (requestedCount < 0)
		startAddress += requestedCount * 32;

	lua_createtable(state, (int)count, 0);
	for (i = 0; i < count; i++)
	{
		unsigned char pixels[32];
		lua_Integer currentAddress = startAddress + i * 32;
		int byteIndex;
		if (!LuaVdpIsCellAddressValid(currentAddress))
			break;
		memcpy(pixels, VRam + currentAddress, sizeof(pixels));
		for (byteIndex = 0; byteIndex < (int)sizeof(pixels); byteIndex += 2)
		{
			unsigned char temp = pixels[byteIndex];
			pixels[byteIndex] = pixels[byteIndex + 1];
			pixels[byteIndex + 1] = temp;
		}
		LuaTilePush(state, pixels);
		lua_rawseti(state, -2, ++readCount);
	}
	lua_pushinteger(state, readCount);
	lua_insert(state, -2);
	return 2;
}

static int LuaVdpWriteCellAt(lua_State *state, lua_Integer address, int tileIndex)
{
	LuaTile *tile;
	unsigned char pixels[32];
	int byteIndex;
	if (!LuaVdpIsCellAddressValid(address))
		return 0;
	tile = LuaTileCheck(state, tileIndex);
	memcpy(pixels, tile->pixels, sizeof(pixels));
	for (byteIndex = 0; byteIndex < (int)sizeof(pixels); byteIndex += 2)
	{
		unsigned char temp = pixels[byteIndex];
		pixels[byteIndex] = pixels[byteIndex + 1];
		pixels[byteIndex + 1] = temp;
	}
	memcpy(VRam + address, pixels, sizeof(pixels));
	VRam_Flag = 1;
	return 1;
}

static int LuaBindVdpWriteCell(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	int dataIndex = 2;
	int count = 0;
	int i;

	LuaVdpIsAvailable(state);
	if (!LuaVdpIsCellAddressValid(address))
		return luaL_argerror(state, 1, "address must point to tile pattern data inside VRAM");
	if (luaL_testudata(state, dataIndex, "kmod.tile"))
	{
		if (LuaVdpWriteCellAt(state, address, dataIndex))
			count++;
		lua_pushinteger(state, count);
		return 1;
	}
	if (!lua_istable(state, dataIndex))
		return luaL_argerror(state, dataIndex, "expected tile userdata or a table of tiles");

	for (i = 1; ; i++)
	{
		lua_rawgeti(state, dataIndex, i);
		if (lua_isnil(state, -1))
		{
			lua_pop(state, 1);
			break;
		}
		if (!luaL_testudata(state, -1, "kmod.tile"))
		{
			lua_pop(state, 1);
			break;
		}
		if (LuaVdpWriteCellAt(state, address, -1))
			count++;
		else
		{
			lua_pop(state, 1);
			break;
		}
		address += 32;
		lua_pop(state, 1);
	}
	lua_pushinteger(state, count);
	return 1;
}

static int LuaBindVdpReadPalette(lua_State *state)
{
	lua_Integer firstLine = 0;
	lua_Integer requestedCount = lua_isnoneornil(state, 2) ? 4 : luaL_checkinteger(state, 2);
	lua_Integer count;
	lua_Integer i;

	LuaVdpIsAvailable(state);
	if (!lua_isnoneornil(state, 1))
	{
		lua_Integer line = luaL_checkinteger(state, 1);
		if (line < 1 || line > LUA_VDP_CRAM_LINE_COUNT)
			return luaL_argerror(state, 1, "palette line must be between 1 and 4");
		firstLine = line - 1;
	}
	if (requestedCount < 0)
	{
		firstLine += requestedCount;
		requestedCount = -requestedCount;
	}
	if (firstLine < 0)
		firstLine = 0;
	if (firstLine >= LUA_VDP_CRAM_LINE_COUNT || requestedCount <= 0)
		count = 0;
	else
	{
		count = requestedCount;
		if (count > LUA_VDP_CRAM_LINE_COUNT - firstLine)
			count = LUA_VDP_CRAM_LINE_COUNT - firstLine;
	}

	lua_createtable(state, (int)count, 0);
	for (i = 0; i < count; i++)
	{
		LuaPalettePush(state, CRam + (firstLine + i) * LUA_VDP_CRAM_LINE_SIZE);
		lua_rawseti(state, -2, i + 1);
	}
	lua_pushinteger(state, count);
	lua_insert(state, -2);
	return 2;
}

static int LuaVdpWritePaletteAt(lua_State *state, lua_Integer line, int paletteIndex)
{
	const unsigned short *colors;
	int i;	
	if (line < 0 || line >= LUA_VDP_CRAM_LINE_COUNT)
		return 0;
	colors = LuaPaletteCheck(state, paletteIndex)->colors;
	for (i = 0; i < 16; i++)
	{
		if (colors[i] != 0xFFFF)
			memcpy(CRam + line * LUA_VDP_CRAM_LINE_SIZE + i * 2, &colors[i], sizeof(colors[i]));
	}
	CRam_Flag = 1;
	return 1;
}

static int LuaBindVdpWritePalette(lua_State *state)
{
	lua_Integer line = 0;
	int dataIndex = 1;
	int count = 0;
	int i;

	LuaVdpIsAvailable(state);
	if (lua_isinteger(state, 1))
	{
		line = luaL_checkinteger(state, 1) - 1;
		dataIndex = 2;
	}
	if (luaL_testudata(state, dataIndex, "kmod.palette"))
	{
		if (LuaVdpWritePaletteAt(state, line, dataIndex))
			count++;
		lua_pushinteger(state, count);
		return 1;
	}
	if (!lua_istable(state, dataIndex))
		return luaL_argerror(state, dataIndex, "expected palette userdata or a table of palettes");

	for (i = 1; ; i++)
	{
		lua_rawgeti(state, dataIndex, i);
		if (lua_isnil(state, -1))
		{
			lua_pop(state, 1);
			break;
		}
		if (!luaL_testudata(state, -1, "kmod.palette"))
		{
			lua_pop(state, 1);
			break;
		}
		if (LuaVdpWritePaletteAt(state, line, -1))
			count++;
		else
		{
			lua_pop(state, 1);
			break;
		}
		line++;
		lua_pop(state, 1);
	}
	lua_pushinteger(state, count);
	return 1;
}

static int LuaBindVdpReadWordVram(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	unsigned short value;
	LuaVdpIsAvailable(state);
	if (address < 0 || (unsigned long long)address + 2 > LUA_VDP_VRAM_SIZE)
		return luaL_argerror(state, 1, "word address is outside VRAM");
	memcpy(&value, VRam + address, sizeof(value));
	lua_pushinteger(state, value);
	return 1;
}

static int LuaBindVdpReadWordVsram(lua_State *state)
{
	lua_Integer address = luaL_checkinteger(state, 1);
	unsigned short value;
	LuaVdpIsAvailable(state);
	if (address < 0 || (unsigned long long)address + 2 > LUA_VDP_VSRAM_SIZE)
		return luaL_argerror(state, 1, "word address is outside VSRAM");
	memcpy(&value, VSRam + address, sizeof(value));
	lua_pushinteger(state, value);
	return 1;
}

static int LuaBindVdpReadRegister(lua_State *state)
{
	lua_Integer reg = luaL_checkinteger(state, 1);
	unsigned int value;

	LuaVdpIsAvailable(state);
	if (reg < 0 || reg > 23)
		return luaL_argerror(state, 1, "VDP register index must be between 0 and 23");

	switch (reg)
	{
	case 0: value = VDP_Reg.Set1; break;
	case 1: value = VDP_Reg.Set2; break;
	case 2: value = VDP_Reg.Pat_ScrA_Adr; break;
	case 3: value = VDP_Reg.Pat_Win_Adr; break;
	case 4: value = VDP_Reg.Pat_ScrB_Adr; break;
	case 5: value = VDP_Reg.Spr_Att_Adr; break;
	case 6: value = VDP_Reg.Reg6; break;
	case 7: value = VDP_Reg.BG_Color; break;
	case 8: value = VDP_Reg.Reg8; break;
	case 9: value = VDP_Reg.Reg9; break;
	case 10: value = VDP_Reg.H_Int; break;
	case 11: value = VDP_Reg.Set3; break;
	case 12: value = VDP_Reg.Set4; break;
	case 13: value = VDP_Reg.H_Scr_Adr; break;
	case 14: value = VDP_Reg.Reg14; break;
	case 15: value = VDP_Reg.Auto_Inc; break;
	case 16: value = VDP_Reg.Scr_Size; break;
	case 17: value = VDP_Reg.Win_H_Pos; break;
	case 18: value = VDP_Reg.Win_V_Pos; break;
	case 19: value = VDP_Reg.DMA_Lenght_L; break;
	case 20: value = VDP_Reg.DMA_Lenght_H; break;
	case 21: value = VDP_Reg.DMA_Src_Adr_L; break;
	case 22: value = VDP_Reg.DMA_Src_Adr_M; break;
	default: value = VDP_Reg.DMA_Src_Adr_H; break;
	}

	lua_pushinteger(state, value);
	return 1;
}

void lua_bind_vdp_register(lua_State *state)
{
	int vdpIndex;
	int signaturesIndex;

	lua_createtable(state, 0, 6);
	vdpIndex = lua_absindex(state, -1);
	lua_createtable(state, 0, 6);
	signaturesIndex = lua_absindex(state, -1);
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "readcell", LuaBindVdpReadCell, "(address[, count])");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "writecell", LuaBindVdpWriteCell, "(address[, tiles])");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "readpalette", LuaBindVdpReadPalette, "([line][, count])");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "writepalette", LuaBindVdpWritePalette, "([line,] palettes)");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "readwordvram", LuaBindVdpReadWordVram, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "readwordvsram", LuaBindVdpReadWordVsram, "(address)");
	KMOD_LUA_BIND_FUNCTION(state, vdpIndex, signaturesIndex, "readregister", LuaBindVdpReadRegister, "(register)");
	lua_pushvalue(state, signaturesIndex);
	lua_setfield(state, vdpIndex, "signatures");
	lua_pop(state, 1);
	lua_setglobal(state, "vdp");
}
