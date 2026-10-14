-- Smoke-test every method currently exposed by emu.
assert(type(emu) == "table", "emu module is unavailable")
assert(emu == gens, "gens should be an alias of emu")
assert(type(emu.signatures) == "table", "emu signature metadata is unavailable")

local function checkMethod(name, result, expectedType)
	assert(type(emu[name]) == "function", "emu." .. name .. " is unavailable")
	assert(type(emu.signatures[name]) == "string", "signature missing for emu." .. name)
	assert(type(result) == expectedType,
		"emu." .. name .. " should return " .. expectedType .. ", got " .. type(result))
	kmod.log("PASS emu." .. name .. emu.signatures[name])
end

checkMethod("framecount", emu.framecount(), "number")
checkMethod("ispaused", emu.ispaused(), "boolean")
checkMethod("getframeskip", emu.getframeskip(), "number")

local country = emu.getcountry()
assert(country == nil or type(country) == "string", "emu.getcountry() should return a string or nil")
checkMethod("getcountry", country, country == nil and "nil" or "string")

local romSize = emu.getromsize()
checkMethod("getromsize", romSize, "number")
assert(romSize >= 0, "emu.getromsize() returned a negative size")

local romName = emu.getromname()
assert(romName == nil or type(romName) == "string", "emu.getromname() should return a string or nil")
checkMethod("getromname", romName, romName == nil and "nil" or "string")

local romHeader = emu.getromheader()
assert(romHeader == nil or type(romHeader) == "table", "emu.getromheader() should return a table or nil")
checkMethod("getromheader", romHeader, romHeader == nil and "nil" or "table")
if romHeader ~= nil then
	for _, field in ipairs({"console", "copyright", "name", "type", "version", "io_support", "description", "countries"}) do
		assert(type(romHeader[field]) == "string",
			"ROM header field '" .. field .. "' should be a string")
	end
	assert(type(romHeader.checksum) == "number", "ROM header checksum should be numeric")
end

local visibleLines = emu.getvisiblelines()
assert(visibleLines == nil or type(visibleLines) == "number",
	"emu.getvisiblelines() should return a number or nil")
checkMethod("getvisiblelines", visibleLines, visibleLines == nil and "nil" or "number")

local systems = emu.getsystems()
checkMethod("getsystems", systems, "table")
for _, name in ipairs({"genesis", "sega_cd", "x32"}) do
	assert(type(systems[name]) == "boolean",
		"emu.getsystems() field '" .. name .. "' should be a boolean")
end

kmod.log("All emu method tests passed.")
