-- Test script-owned palettes and read-only Genesis CRAM access.
assert(type(palette) == "table", "palette module is unavailable")
assert(palette == pal, "pal should alias palette")
assert(type(palette.signatures) == "table", "palette signature metadata is unavailable")

local methods = {"new", "create", "getcolor", "readcolor", "setcolor", "writecolor", "rawdata", "readcramcolor"}
for _, name in ipairs(methods) do
	assert(type(palette[name]) == "function", "palette." .. name .. " is unavailable")
	assert(type(palette.signatures[name]) == "string", "signature missing for palette." .. name)
end

local colors = palette.new()
assert(type(colors) == "userdata", "palette.new should return palette userdata")
assert(palette.getcolor(colors, 1) == nil, "new palettes should start transparent")

palette.setcolor(colors, 1, {r = 255, g = 128, b = 0, a = 255})
local color = palette.getcolor(colors, 1)
assert(color.r == 224 and color.g == 128 and color.b == 0 and color.a == 255,
	"palette channels should round down to Genesis 3-bit RGB")
local raw = palette.rawdata(colors)
assert(type(raw) == "table" and raw[1] == 0x00 and raw[2] == 0x8E,
	"rawdata should expose the first color as high-byte-first Genesis CRAM data")

palette.writecolor(colors, 2, {r = 0, g = 0, b = 255, a = 255})
assert(palette.readcolor(colors, 2).b == 224, "read/write aliases should work")
palette.setcolor(colors, 1, nil)
assert(palette.getcolor(colors, 1) == nil, "nil should clear a palette entry")

local cramColor = palette.readcramcolor(0, 0)
assert(type(cramColor) == "table", "readcramcolor should return channel values")
for _, channel in ipairs({"r", "g", "b", "a"}) do
	assert(type(cramColor[channel]) == "number", "CRAM color channel should be numeric")
	assert(cramColor[channel] >= 0 and cramColor[channel] <= 255, "CRAM channel is out of range")
end
assert(cramColor.a == 255, "CRAM alpha should be opaque")

local ok = pcall(palette.readcramcolor, 4, 0)
assert(not ok, "synthetic debug palettes must not be exposed as CRAM")
ok = pcall(palette.readcramcolor, 0, 16)
assert(not ok, "CRAM color index 16 should be rejected")
ok = pcall(palette.getcolor, colors, 0)
assert(not ok, "palette color index 0 should be rejected")

kmod.log("All palette method tests passed.")
