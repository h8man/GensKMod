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

local romSize = emu.getromsize()
checkMethod("getromsize", romSize, "number")
assert(romSize >= 0, "emu.getromsize() returned a negative size")

local systems = emu.getsystems()
checkMethod("getsystems", systems, "table")
for _, name in ipairs({"genesis", "sega_cd", "32x"}) do
	assert(type(systems[name]) == "boolean",
		"emu.getsystems() field '" .. name .. "' should be a boolean")
end

kmod.log("All emu method tests passed.")
