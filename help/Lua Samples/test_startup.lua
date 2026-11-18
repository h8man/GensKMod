-- Run this script directly from the command line to load the ROM and open its debug windows.
local rom_path = "..\\..\\..\\..\\..\\[SEGA]\\Altered Beast (USA, Europe).zip"

assert(type(emu) == "table" and type(emu.loadrom) == "function", "emu.loadrom is unavailable")
assert(type(kmod) == "table" and type(kmod.set_window_visible) == "function",
	"KMod window visibility API is unavailable")
assert(type(kmod.set_window_position) == "function", "KMod window positioning API is unavailable")
assert(emu.loadrom(rom_path), "ROM failed to load: " .. rom_path)

local systems = emu.getsystems()
assert(systems.genesis, "The test ROM did not start as a Genesis game")
assert(kmod.set_window_visible("genesis_68k", true), "Unable to open the 68K debug window")
assert(kmod.set_window_visible("genesis_vdp", true), "Unable to open the VDP debug window")
assert(kmod.set_window_visible("lua_script", true), "Unable to show the Lua Script window")
assert(kmod.set_window_position("genesis_68k", 500, 100), "Unable to position the 68K debug window")
assert(kmod.set_window_position("genesis_vdp", 100, 350), "Unable to position the VDP debug window")
assert(kmod.set_window_position("lua_script", 1200, 350), "Unable to position the Lua Script window")
kmod.log("Startup script loaded the ROM and arranged the 68K, VDP, and Lua windows")
