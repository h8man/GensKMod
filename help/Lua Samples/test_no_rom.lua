-- Run this smoke test with no ROM open.
local systems = emu.getsystems()

assert(emu.getromname() == nil, "ROM name should be nil with no ROM")
assert(emu.getromheader() == nil, "ROM header should be nil with no ROM")
assert(emu.getromsize() == 0, "ROM size should be zero with no ROM")
assert(not systems.genesis and not systems.sega_cd and not systems.x32,
	"no emulated system should be active")
assert(not pcall(memory.readbyte, 0), "memory access should require an emulated system")
assert(not pcall(joypad.get, 1), "joypad access should require an emulated system")
assert(not pcall(savestate.save, 0), "savestates should require an emulated system")
assert(not pcall(vdp.readregister, 0), "VDP access should require Genesis")
assert(not pcall(palette.readcramcolor, 0, 0), "live CRAM access should require Genesis")

kmod.log("No-ROM Lua smoke test passed")