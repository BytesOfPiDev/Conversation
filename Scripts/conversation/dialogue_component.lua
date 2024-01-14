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

-- @brief Executes the script attached to a dialogue.
--
-- Nothing happens if no script is found that matches the Id..
--
-- @param nodeId The Id of the node whose script needs to be run
function lib.ScriptDialogueComponent:RunDialogueScript(nodeId)
	Debug.Log("RunDialogueScript")
	local nodeFunc = self[nodeId]
	if type(nodeFunc) == "function" then
		return nodeFunc()
	end
end

-------------------------------------------------------------------------------
-- @brief Runs and returns the return of the given node's condition script(s).
--
-- @note If no node is found, we assume there are no conditions and returning
--       true.
-- @returns true if all conditions are satisfied. Otherwise, returns false.
-------------------------------------------------------------------------------
function lib.ScriptDialogueComponent:IsAvailable(nodeId)
	local conditionFunc = self[nodeId]

	-- Conditions that are functions must return a boolean type.
	if type(conditionFunc) == "function" then
		local result = conditionFunc()
		if type(result) == "boolean" then
			return result
		end

		-- We fail the availability check because we didn't a boolean.
		return false
	end

	-- We return true if no condition function was found because dialogues are available by default.
	-- No conditon means we should always return true.
	Debug.Log(
		"IsAvailable returning 'true' since a condition function for '" .. tostring(nodeId) .. "' could not be found."
	)
	return true
end

-------------------------------------------------------------------------------
-- @brief Helper for adding a condition script to a dialogue.
--
-- @param dialogueNodeName The name of the node to add the condition to.
-- @param conditionFunction A function returning a boolean result.
-------------------------------------------------------------------------------
function lib.ScriptDialogueComponent:AddCondition(dialogueNodeName, conditionFunction)
	if dialogueNodeName == nil then
		return
	end

	-- Currently, we only accept function types
	if type(conditionFunction) == "function" then
		self.conditions[dialogueNodeName] = conditionLib.ConditionScript:New(self.entityId, conditionFunction)
		return
	end

	Debug.Log("[dialogue_component.lua] The condition function given is not a function. No condition will be added.")
end

return lib
