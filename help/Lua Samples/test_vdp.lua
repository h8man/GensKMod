-- Test VDP accessors. Write checks briefly modify live VRAM/CRAM, then restore the original data.
assert(type(vdp) == "table", "vdp module is unavailable")

local methods = {"readcell", "writecell", "readpalette", "writepalette", "readwordvram", "readwordvsram", "readregister"}
for _, name in ipairs(methods) do
	assert(type(vdp[name]) == "function", "vdp." .. name .. " is unavailable")
	assert(type(vdp.signatures[name]) == "string", "signature missing for vdp." .. name)
end

local ok = pcall(vdp.readwordvram, -1)
assert(not ok, "negative VRAM address should be rejected")
ok = pcall(vdp.readwordvram, 64000)
assert(not ok, "VRAM word beyond allocated range should be rejected")
ok = pcall(vdp.readwordvsram, 256)
assert(not ok, "VSRAM word beyond allocated range should be rejected")
ok = pcall(vdp.readregister, -1)
assert(not ok, "negative VDP register index should be rejected")
ok = pcall(vdp.readregister, 24)
assert(not ok, "VDP register index beyond 23 should be rejected")
ok = pcall(vdp.readpalette, 5, 1)
assert(not ok, "palette lines beyond 4 should not produce data")

local count, tiles = vdp.readcell(0, 0)
assert(type(count) == "number" and count == 0, "zero-count cell read should return an empty list")
assert(type(tiles) == "table", "readcell should return a tile list")

local paletteCount, palettes = vdp.readpalette(1, 0)
assert(paletteCount == 0 and type(palettes) == "table", "zero-count palette read should return an empty list")

local vramWord = vdp.readwordvram(0)
assert(type(vramWord) == "number" and vramWord >= 0 and vramWord <= 0xFFFF, "VRAM word read should return an unsigned word")
local vsramWord = vdp.readwordvsram(0)
assert(type(vsramWord) == "number" and vsramWord >= 0 and vsramWord <= 0xFFFF, "VSRAM word read should return an unsigned word")
local vdpRegister = vdp.readregister(2)
assert(type(vdpRegister) == "number" and vdpRegister >= 0 and vdpRegister <= 0xFF,
	"VDP register read should return an unsigned byte")

local cellAddress = 0x0000
local cellCount, cells = vdp.readcell(cellAddress, 1)
assert(cellCount == 1, "expected a tile pattern at test VRAM address " .. string.format("0x%04X", cellAddress))
local originalTile = cells[1]
assert(type(originalTile) == "userdata", "readcell should return tile userdata")

local originalPixel = tile.getpixel(originalTile, 1, 1)
assert(originalPixel >= 0 and originalPixel <= 15, "read tile pixel is out of range")
local changedTile = tile.new()
for y = 1, 8 do
	for x = 1, 8 do
		tile.setpixel(changedTile, x, y, tile.getpixel(originalTile, x, y))
	end
end
tile.setpixel(changedTile, 1, 1, (originalPixel + 1) % 16)

local cellTestOk, cellTestError = pcall(function()
	assert(vdp.writecell(cellAddress, originalTile) == 1, "writecell should accept a tile userdata")
	assert(vdp.writecell(cellAddress, {changedTile}) == 1, "writecell should accept a tile list")
	local count, cells = vdp.readcell(cellAddress, 1)
	assert(count == 1 and tile.getpixel(cells[1], 1, 1) == (originalPixel + 1) % 16,
		"VRAM cell write/read round trip failed")
end)
local restoreCellCount = vdp.writecell(cellAddress, originalTile)
assert(restoreCellCount == 1, "failed to restore the original VRAM cell")
assert(cellTestOk, cellTestError)

local originalPaletteCount, originalPalettes = vdp.readpalette(1, 1)
assert(type(originalPaletteCount) == "number", "readpalette should return the count first; got " .. type(originalPaletteCount))
assert(originalPaletteCount == 1, "readpalette(1, 1) returned " .. tostring(originalPaletteCount) .. " lines")
assert(type(originalPalettes) == "table", "readpalette should return a palette list second; got " .. type(originalPalettes))
assert(type(originalPalettes[1]) == "userdata",
	"readpalette should return palette userdata; got " .. type(originalPalettes[1]))
local originalPalette = originalPalettes[1]
local changedPalette = palette.new()
for i = 1, 16 do
	palette.setcolor(changedPalette, i, palette.getcolor(originalPalette, i))
end
palette.setcolor(changedPalette, 1, {r = 255, g = 0, b = 0, a = 255})

local paletteTestOk, paletteTestError = pcall(function()
	assert(vdp.writepalette(1, originalPalette) == 1, "writepalette should accept palette userdata")
	assert(vdp.writepalette(1, {changedPalette}) == 1, "writepalette should accept a palette list")
	local count, lines = vdp.readpalette(1, 1)
	assert(count == 1, "palette readback should return one line")
	local color = palette.getcolor(lines[1], 1)
	assert(color ~= nil and color.r == 224 and color.g == 0 and color.b == 0,
		"CRAM palette write/read round trip failed")
end)
local restorePaletteCount = vdp.writepalette(1, originalPalette)
assert(restorePaletteCount == 1, "failed to restore the original CRAM palette line")
assert(paletteTestOk, paletteTestError)

kmod.log("All VDP read/write and bounds tests passed; modified VRAM/CRAM data was restored.")
