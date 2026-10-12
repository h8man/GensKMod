-- Test standalone tile objects. This module does not read from or write to VRAM.
assert(type(tile) == "table", "tile module is unavailable")

local methods = {"new", "create", "getpixel", "readpixel", "setpixel", "writepixel", "rawdata"}
for _, name in ipairs(methods) do
	assert(type(tile[name]) == "function", "tile." .. name .. " is unavailable")
	assert(type(tile.signatures[name]) == "string", "signature missing for tile." .. name)
end

local t = tile.new()
assert(type(t) == "userdata", "tile.new should return tile userdata")
assert(tile.getpixel(t, 1, 1) == 0, "new tiles should be zero-filled")

tile.setpixel(t, 1, 1, 10)
tile.setpixel(t, 2, 1, 5)
tile.writepixel(t, 1, 2, 3)
assert(tile.readpixel(t, 1, 1) == 10, "pixel read alias should work")
assert(tile.getpixel(t, 2, 1) == 5, "low nibble pixel read failed")
assert(tile.getpixel(t, 1, 2) == 3, "row stride or write alias is incorrect")

local raw = tile.rawdata(t)
assert(type(raw) == "table" and #raw == 32, "rawdata should contain 32 bytes")
assert(raw[1] == 0xA5 and raw[5] == 0x30, "pixels should use high-nibble-first packed rows")

local ok = pcall(tile.getpixel, t, 0, 1)
assert(not ok, "x coordinate 0 should be rejected")
ok = pcall(tile.setpixel, t, 1, 9, 1)
assert(not ok, "y coordinate 9 should be rejected")
ok = pcall(tile.setpixel, t, 1, 1, 16)
assert(not ok, "pixel color 16 should be rejected")

kmod.log("All tile method tests passed.")
