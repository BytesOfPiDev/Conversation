-- dialogue_component_script.lua

local conditionLib = require("conversation.condition")

local lib = {}

lib.ScriptDialogueComponent = {
	conditions = {},
	dialogueComponentNotificationHandler = nil,
	availabilityRequestBusHandler = nil,
}

function lib.ScriptDialogueComponent:New()
	local o = {}

	self.__index = self
	return setmetatable(o, self)
end

-- Performs the basic setup needed to establish communication between us and the DialogueComponent on the entity we're attached to.
function lib.ScriptDialogueComponent:ActivateConversationScript()
	assert(self.InitConversationScript ~= nil, "Missing required InitConversationScript function!")

	if self.InitConversationScript ~= nil then
		self:InitConversationScript()
	end

	self.dialogueComponentNotificationHandler = DialogueComponentNotificationBus.Connect(self, self.entityId)
	self.availabilityRequestBusHandler = AvailabilityRequestBus.Connect(self, self.entityId)
	self.companionScriptRequestHandler = DialogueScriptRequestBus.Connect(self, self.entityId)
end

function lib.ScriptDialogueComponent:DeactivateConversationScript()
	if self.dialogueComponentNotificationHandler ~= nil then
		self.dialogueComponentNotificationHandler:Disconnect()
		self.dialogueComponentNotificationHandler = nil
	end

	if self.availabilityRequestBusHandler ~= nil then
		self.availabilityRequestBusHandler:Disconnect()
		self.availabilityRequestBusHandler = nil
	end

	if self.companionScriptRequestHandler ~= nil then
		self.companionScriptRequestHandler:Disconnect()
		self.companionScriptRequestHandler = nil
	end
end

function lib.ScriptDialogueComponent:OnConversationStarted(initiatingEntityId) end

function lib.ScriptDialogueComponent:OnConversationEnded() end

function lib.ScriptDialogueComponent:OnConversationAborted() end

function lib.ScriptDialogueComponent:GetOwnerEntityId()
	return self.entityId
end

function lib.ScriptDialogueComponent:OnDialogue(dialogue, availableResponses) end

function lib.ScriptDialogueComponent:RunDialogueScript(nodeId)
	Debug.Log("RunDialogueScript")
	local nodeFunc = self[nodeId]
	if type(nodeFunc) == "function" then
		return nodeFunc()
	end
end

function lib.ScriptDialogueComponent:IsAvailable(nodeId)
	local conditionFunc = self[nodeId]
	if type(conditionFunc) == "function" then
		return conditionFunc()
	end

	-- We return true if no condition function was found because dialogues are available by default.
	-- No conditon means we should always return true.
	Debug.Log(
		"IsAvailable returning 'true' since a condition function for '" .. tostring(nodeId) .. "' could not be found."
	)
	return true
end

function lib.ScriptDialogueComponent:AddCondition(dialogueNodeName, conditionFunction)
	if dialogueNodeName == nil then
		return
	end

	-- Currently, we only accept function types
	if type(conditionFunction) == "function" then
		self.conditions[dialogueNodeName] = conditionLib.ConditionScript:New(self.entityId, conditionFunction)
		return
	end

	Debug.Log("[core.lua] The condition function given is not a function. No condition will be added.")
end

return lib
