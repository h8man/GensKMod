-- Smoke-test read-only emulated joypad state.
assert(type(joypad) == "table", "joypad module is unavailable")
assert(type(joypad.get) == "function", "joypad.get is unavailable")
assert(type(joypad.signatures) == "table", "joypad signature metadata is unavailable")
assert(type(joypad.signatures.get) == "string", "signature missing for joypad.get")

local buttons = {"up", "down", "left", "right", "a", "b", "c", "x", "y", "z", "start", "mode"}
for player = 1, 2 do
	local state = joypad.get(player)
	assert(type(state) == "table", "joypad.get should return a table")
	for _, button in ipairs(buttons) do
		assert(type(state[button]) == "boolean",
			"joypad.get(" .. player .. ")." .. button .. " should be a boolean")
	end
end

local ok = pcall(joypad.get, 0)
assert(not ok, "player 0 should be rejected")
ok = pcall(joypad.get, 9)
assert(not ok, "players beyond available slots should be rejected")

kmod.log("PASS joypad.get" .. joypad.signatures.get)
kmod.log("All joypad method tests passed.")
