-- Smoke-test the read-only memory API and its supported ranges.
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
	"readbyterange"
}
for _, name in ipairs(methods) do
	assert(type(memory[name]) == "function", "memory." .. name .. " is unavailable")
	assert(type(memory.signatures[name]) == "string", "signature missing for memory." .. name)
	kmod.log("PASS memory." .. name .. memory.signatures[name])
end

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

kmod.log("All memory method tests passed.")
