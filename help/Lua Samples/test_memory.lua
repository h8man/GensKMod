-- Smoke-test memory reads/writes and mapped ranges. Writes below restore the original work RAM bytes.
assert(type(memory) == "table", "memory module is unavailable")
assert(type(memory.signatures) == "table", "memory signature metadata is unavailable")

local methods = {
	"isvalid",
	"readbyte",
	"readbytesigned",
	"readword",
	"readwordsigned",
	"readdword",
	"readdwordsigned",
	"readbyterange",
	"writebyte",
	"writeword",
	"writedword",
	"writebyterange",
	"readbyteunsigned",
	"readwordunsigned",
	"readdwordunsigned",
	"readshort",
	"readshortunsigned",
	"readshortsigned",
	"readlong",
	"readlongunsigned",
	"readlongsigned",
	"writeshort",
	"writelong",
	"getregister",
	"setregister"
}
for _, name in ipairs(methods) do
	assert(type(memory[name]) == "function", "memory." .. name .. " is unavailable")
	assert(type(memory.signatures[name]) == "string", "signature missing for memory." .. name)
	kmod.log("PASS memory." .. name .. memory.signatures[name])
end

local originalD0 = memory.getregister("d0")
assert(type(originalD0) == "number", "unqualified register names should read main 68000 registers")
assert(memory.getregister("main.d0") == originalD0, "main.d0 should alias d0")
assert(memory.getregister("m68k.d0") == originalD0, "m68k.d0 should alias d0")
assert(type(memory.getregister("main.pc")) == "number", "main PC should be readable")
assert(type(memory.getregister("main.sr")) == "number", "main SR should be readable")
local registerWriteOk, registerWriteError = pcall(function()
	memory.setregister("main.d0", originalD0 ~ 0xFFFFFFFF)
	assert(memory.getregister("d0") == (originalD0 ~ 0xFFFFFFFF), "main register write/read round trip failed")
end)
memory.setregister("d0", originalD0)
assert(registerWriteOk, registerWriteError)
if emu.getsystems().sega_cd then
	local originalSubD0 = memory.getregister("sub.d0")
	assert(memory.getregister("s68k.d0") == originalSubD0, "s68k.d0 should alias sub.d0")
	local subRegisterWriteOk, subRegisterWriteError = pcall(function()
		memory.setregister("s68k.d0", originalSubD0 ~ 0xFFFFFFFF)
		assert(memory.getregister("sub.d0") == (originalSubD0 ~ 0xFFFFFFFF), "sub register write/read round trip failed")
	end)
	memory.setregister("sub.d0", originalSubD0)
	assert(subRegisterWriteOk, subRegisterWriteError)
end
assert(memory.getregister("not_a_register") == nil, "unknown register names should return nil")
memory.setregister("not_a_register", 1)

assert(memory.isvalid(-1) == false, "negative addresses must be invalid")
assert(memory.isvalid(0xFF0000), "main 68K RAM should be valid")
assert(memory.isvalid(0xFFFFFF), "last main RAM byte should be valid")
assert(memory.isvalid(0xFFFFFF, 2) == false, "a range past main RAM must be invalid")
assert(memory.isvalid(0xFF0000, 0), "an empty range at the start of main RAM should be valid")
assert(memory.isvalid(0xA00000), "Z80 RAM should be valid")
assert(memory.isvalid(0xA01FFF), "last Z80 RAM byte should be valid")
assert(memory.isvalid(0xA02000) == false, "addresses past Z80 RAM must be invalid")

local systems = emu.getsystems()
if systems.sega_cd then
	assert(memory.isvalid(0x020000), "Sega CD PRG RAM should be valid when Sega CD is active")
	assert(memory.isvalid(0x09FFFF), "last Sega CD PRG RAM byte should be valid")
	assert(memory.isvalid(0x200000), "Sega CD word RAM should be valid when Sega CD is active")
end
if systems.x32 then
	assert(memory.isvalid(0x06000000), "32X RAM should be valid when 32X is active")
	assert(memory.isvalid(0x0603FFFF), "last 32X RAM byte should be valid")
	assert(memory.isvalid(0x06040000) == false, "addresses past 32X RAM must be invalid")
end

local lastRamByte = memory.readbyte(0xFFFFFF)
assert(lastRamByte >= 0 and lastRamByte <= 0xFF, "readbyte should return an unsigned byte")
local signedByte = memory.readbytesigned(0xFFFFFF)
assert(signedByte >= -0x80 and signedByte <= 0x7F, "readbytesigned should return a signed byte")

local emptyRange = memory.readbyterange(0xFF0000, 0)
assert(type(emptyRange) == "table" and #emptyRange == 0, "empty byte range should return an empty table")
local bytes = memory.readbyterange(0xFFFFFF, 1)
assert(#bytes == 1 and bytes[1] == lastRamByte, "readbyterange should return one-based byte values")

local ok = pcall(memory.readword, 0xFFFFFF)
assert(not ok, "reads extending past RAM should fail")
ok = pcall(memory.readbyterange, 0xFF0000, 65537)
assert(not ok, "byte ranges larger than 64 KiB should fail")
ok = pcall(memory.readbyte, 0xA02000)
assert(not ok, "reads outside Z80 RAM should fail")

local testAddress = 0xFF0100
local originalBytes = memory.readbyterange(testAddress, 9)
local writeTestOk, writeTestError = pcall(function()
	memory.writebyte(testAddress, 0xA5)
	assert(memory.readbyte(testAddress) == 0xA5, "writebyte/readbyte round trip failed")
	memory.writeword(testAddress + 1, 0x1234)
	assert(memory.readword(testAddress + 1) == 0x1234, "writeword/readword round trip failed")
	memory.writedword(testAddress + 3, 0x89ABCDEF)
	assert(memory.readdword(testAddress + 3) == 0x89ABCDEF, "writedword/readdword round trip failed")
	assert(memory.writebyterange(testAddress + 7, {0x12, 0x34}) == 2, "writebyterange should return number of bytes written")
	local writtenBytes = memory.readbyterange(testAddress + 7, 2)
	assert(writtenBytes[1] == 0x12 and writtenBytes[2] == 0x34, "writebyterange/readbyterange round trip failed")
	assert(memory.readshort(testAddress + 1) == 0x1234, "short aliases should match word reads")
	assert(memory.readlong(testAddress + 3) == 0x89ABCDEF, "long aliases should match dword reads")
end)
local restoreCount = memory.writebyterange(testAddress, originalBytes)
assert(restoreCount == #originalBytes, "failed to restore original work RAM bytes")
assert(writeTestOk, writeTestError)

assert(emu.getromsize() > 0, "ROM must be loaded to test ROM writes")
local romTestAddress = 0
local originalRomByte = memory.readbyte(romTestAddress)
local romWriteOk, romWriteError = pcall(function()
	local changedRomByte = originalRomByte ~ 0xFF
	memory.writebyte(romTestAddress, changedRomByte)
	assert(memory.readbyte(romTestAddress) == changedRomByte, "ROM write/read round trip failed")
end)
local romRestoreOk, romRestoreError = pcall(memory.writebyte, romTestAddress, originalRomByte)
assert(romRestoreOk, "failed to restore original ROM byte: " .. tostring(romRestoreError))
assert(romWriteOk, romWriteError)
ok = pcall(memory.writebyte, 0xFFFFFF, 0x100)
assert(not ok, "byte writes must reject values larger than 255")
ok = pcall(memory.writeword, 0xFFFFFF, 0x1234)
assert(not ok, "word writes extending past RAM must fail")
ok = pcall(memory.writebyterange, 0xFF0000, {0, 256})
assert(not ok, "byte ranges must reject values outside 0-255")

kmod.log("All memory read/write method tests passed; original work RAM and ROM bytes were restored. Hooks are not included.")
