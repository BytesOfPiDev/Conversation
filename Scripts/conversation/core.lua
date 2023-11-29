-- core.lua

function GetEntityByTag(tag)
	return TagGlobalRequestBus.Event.GetEntityByTag(Crc32(tag));
end

DialogueScript = { owningEntityId = "", firstName = "" }

function DialogueScript:new(o, owningEntityId, firstName)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	self.owningEntityId = owningEntityId or EntityId()
	self.firstName = firstName or ""
	return o
end

function DialogueScript:Run()
	Debug.Log("Base DialogueScript:Run() called.")
end

function DialogueScript:GetOwningEntityId()
	return self.owningEntityId
end

ConditionScript = {}

function ConditionScript:New(o, conditionFunction)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	self.owningEntityId = owningEntityId or EntityId()
	self.conditionFunction = conditionFunction or function()
		return false
	end
	return 0
end

function ConditionScript:IsAvailable()
	return self.conditionFunction()
end

ScriptDialogueComponent = {
	Properties = {
		-- Property definitions
		FirstName = { default = "Janet" },
		LastName = { default = "Doet" },
		SpeakerTag = { default = "" },
	},
	conditions = {},
	dialogues = {},
	scripts = {},
	dialogueComponentNotificationHandler = nil,
	availabilityRequestBusHandler = nil,
}

function ScriptDialogueComponent:Setup()
	-- Activation Code
	assert(self.Configure ~= nil, "Missing required Configure function!")

	if self.Configure ~= nil then
		self:Configure()
	end

	self.dialogueComponentNotificationHandler = DialogueComponentNotificationBus.Connect(self, self.entityId)
	self.availabilityRequestBusHandler = AvailabilityRequestBus.Connect(self, self.entityId)
	Debug.Log(
		"'ConversationGraphName' ScriptComponent activated on entity: '"
			.. GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)
			.. "'"
	)
end

function ScriptDialogueComponent:Shutdown()
	Debug.Log("Shutdown")
	-- Deactivation Code
	if self.dialogueComponentNotificationHandler ~= nil then
		self.dialogueComponentNotificationHandler:Disconnect()
		self.dialogueComponentNotificationHandler = nil
	end
end

function ScriptDialogueComponent:OnConversationStarted(initiatingEntityId) end

function ScriptDialogueComponent:OnConversationEnded() end

function ScriptDialogueComponent:OnConversationAborted() end

function ScriptDialogueComponent:GetOwnerEntityId()
	return self.entityId
end

function ScriptDialogueComponent:OnDialogue(dialogue, availableResponses) end

function ScriptDialogueComponent:GetConditionFunction(conditionId)
	return function() end
end

function ScriptDialogueComponent:CheckCondition(conditionId)
	-- We require a valid conditionId
	-- TODO: More specifically, we need to check that it is the expected C++ type.
	local isConditionIdValid = (type(self.conditions[conditionId]) ~= "nil")
	assert(isConditionIdValid, "ConditionId is nil. A valid ConditionId is required.\n")

	if not isConditionIdValid then
		return false
	end

	-- The table value must be a function
	-- TODO: Maybe allow it to be either a function or boolean value?
	local isConditionFunctionValid = (type(self.conditions[conditionId]) == "function")
	assert(isConditionFunctionValid, ("ConditionId '" .. conditionId .. "' is not a function.\n"))

	-- We need to make sure it returned a boolean.
	local result = self.conditions[conditionId]()
	local resultIsBoolean = (type(result) == "boolean")
	assert(resultIsBoolean, ("Condition '" .. conditionId .. "' did not return a boolean value.\n"))

	if resultIsBoolean then
		return result
	end

	return false
end

function ScriptDialogueComponent:IsAvailable(conditionId)
	Debug.Log("IsAvailable called in lua script: conditionid [" .. conditionId .. "]")

	if type(self.conditions[conditionId]) == "function" then
		return self.conditions[conditionId]()
	end

	-- We return true if no condition function was found because dialogues are available by default.
	-- No conditon means we should always return true.
	Debug.Log("IsAvailable returning 'true' since there are no conditions.")
	return true
end

function ScriptDialogueComponent:AddCondition(conditionId, conditionFunction)
	Debug.Log("AddCondition called in lua script.")
	if conditionFunction == nil then
		Debug.Log("Condition function '" .. conditionId .. "' is nil.\n")
		return
	end

	self.conditions[conditionId] = conditionFunction
end

function ScriptDialogueComponent:new(o, firstName, lastName)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	self.Properties.FirstName = firstName or ""
	self.Properties.LastName = lastName or ""
	return o
end

Debug.Log("Convesation's 'core.lua' loaded.\n")
