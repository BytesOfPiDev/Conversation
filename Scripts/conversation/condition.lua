-- condition_script.lua

local lib = {}

lib.ConditionScript = {}

function lib.ConditionScript:New(owningEntityId, conditionFunction)
	local o = {}
	setmetatable(o, { __index = self })

	o.owningEntityId = owningEntityId or EntityId()
	o.conditionFunction = conditionFunction

	return o
end

function lib.ConditionScript:IsAvailable()
	return self.conditionFunction()
end

return lib
