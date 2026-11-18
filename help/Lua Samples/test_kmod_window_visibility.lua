-- Run with the Lua Script Editor open, with or without a ROM.
-- Names: message, watchers, layers, plane_explorer, plane_composite, lua_script,
-- genesis_68k, genesis_z80, genesis_vdp, genesis_vdp_registers, genesis_sprites,
-- genesis_ym2612, genesis_psg, sega_cd_68k, sega_cd_cdc, sega_cd_gfx,
-- sega_cd_registers, 32x_main_sh2, 32x_sub_sh2, 32x_vdp, 32x_registers.
assert(type(kmod.set_window_visible) == "function", "KMod window visibility API is unavailable")
assert(kmod.signatures.set_window_visible == "(name, visible) -> boolean",
	"KMod window visibility signature is incorrect")
assert(type(kmod.set_window_position) == "function", "KMod window positioning API is unavailable")
assert(kmod.signatures.set_window_position == "(name, x, y) -> boolean",
	"KMod window positioning signature is incorrect")

assert(kmod.set_window_visible("lua_script", false), "Unable to hide the Lua Script window")
assert(kmod.set_window_visible("lua_script", true), "Unable to show the Lua Script window")
assert(kmod.set_window_position("lua_script", 40, 40), "Unable to position the Lua Script window")
assert(not pcall(kmod.set_window_visible, "unknown_window", true), "Unknown window name should be rejected")
assert(not pcall(kmod.set_window_visible, "lua_script", 1), "Visibility must be a boolean")
assert(not pcall(kmod.set_window_position, "unknown_window", 0, 0), "Unknown window name should be rejected")
assert(not pcall(kmod.set_window_position, "lua_script", 1.5, 0), "Window coordinates must be integers")

local systems = emu.getsystems()

if systems.genesis then
	assert(kmod.set_window_visible("genesis_68k", true),
		"Unable to show the Genesis debugger window")
	assert(kmod.set_window_visible("genesis_vdp", true),
		"Unable to show the Genesis VDP window")
	assert(kmod.set_window_position("genesis_68k", 40, 100),
		"Unable to position the Genesis debugger window")
	assert(kmod.set_window_position("genesis_vdp", 400, 100),
		"Unable to position the Genesis VDP window")
elseif not systems.sega_cd and not systems.x32 then
	assert(not kmod.set_window_visible("genesis_vdp", true),
		"Genesis VDP window should not be shown without an emulated system")
end

kmod.log("KMod individual window visibility test passed")
