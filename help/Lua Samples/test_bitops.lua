-- Test Lua 5.5 native integer bitwise operators; no compatibility bit module is required.
local function check(name, actual, expected)
	assert(actual == expected,
		name .. ": expected " .. tostring(expected) .. ", got " .. tostring(actual))
	kmod.log("PASS " .. name)
end

check("AND", 0xF0 & 0x3C, 0x30)
check("OR", 0xF0 | 0x0F, 0xFF)
check("XOR", 0xF0 ~ 0x3C, 0xCC)
check("NOT with 8-bit mask", (~0x0F) & 0xFF, 0xF0)
check("left shift", 0x01 << 8, 0x100)
check("right shift", 0xF0 >> 4, 0x0F)
check("combined mask", (0xA5 & 0xF0) | (0x0A & 0x0F), 0xAA)

kmod.log("All native bitwise operator tests passed.")
