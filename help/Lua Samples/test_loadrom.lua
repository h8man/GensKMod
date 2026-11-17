-- Run with no ROM open. Relative paths resolve from the emulator directory.
local rom_path = "..\\..\\..\\..\\..\\[SEGA]\\Altered Beast (USA, Europe).zip"

assert(type(emu) == "table", "emu module is unavailable")
assert(type(emu.loadrom) == "function", "emu.loadrom is unavailable")
assert(emu.signatures.loadrom == "(path)", "emu.loadrom signature is incorrect")

assert(emu.loadrom(rom_path), "ROM failed to load")
kmod.log("PASS emu.loadrom(" .. rom_path .. ") loaded")
