-- Group aliases that reference the same module table, such as emu and gens.
local modulesByTable = {}
local globalFunctions = {}

local function addModule(name, value)
	if type(value) ~= "table" then
		return
	end

	local module = modulesByTable[value]
	if not module then
		module = {aliases = {}, exports = value}
		modulesByTable[value] = module
	end
	module.aliases[name] = true
end

-- Find modules exposed as globals and list standalone global functions separately.
for name, value in pairs(_G) do
	if name ~= "_G" then
		if type(value) == "table" then
			addModule(name, value)
		elseif type(value) == "function" then
			globalFunctions[#globalFunctions + 1] = name
		end
	end
end

-- Include modules loaded with require, even when they are not global variables.
if type(package) == "table" and type(package.loaded) == "table" then
	for name, value in pairs(package.loaded) do
		if name ~= "_G" then
			addModule(tostring(name), value)
		end
	end
end

local modules = {}
for _, module in pairs(modulesByTable) do
	module.names = {}
	for name in pairs(module.aliases) do
		module.names[#module.names + 1] = name
	end
	module.aliases = nil
	table.sort(module.names)
	modules[#modules + 1] = module
end

table.sort(globalFunctions)
table.sort(modules, function(left, right)
	return left.names[1] < right.names[1]
end)

-- Print globals

kmod.log("Global functions")
for _, name in ipairs(globalFunctions) do
	kmod.log("\t" .. name .. "(?)")
end

-- Print modules 
for _, module in ipairs(modules) do
	kmod.log(table.concat(module.names, " / "))
	-- collect func for sorting
	local functionNames = {}
	for name, value in pairs(module.exports) do
		if type(name) == "string" and type(value) == "function" then
			functionNames[#functionNames + 1] = name
		end
	end

	table.sort(functionNames)
	local signatures = module.exports.signatures

	for _, name in ipairs(functionNames) do
		-- Display argument signatures when a module provides them.
		local signature = type(signatures) == "table" and signatures[name]
		kmod.log("\t" .. name .. (type(signature) == "string" and signature or "(?)"))
	end
end
