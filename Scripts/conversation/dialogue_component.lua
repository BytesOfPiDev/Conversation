-- dialogue_component_script.lua
local ScriptDialogueComponent = {}
local camera_target_tag = Crc32("camera_target")

function ScriptDialogueComponent:New()
    local o = {
        Properties = {
            EnableDebug = { default = false, description = "Enable debug logging" }
        },
        conditions = {},
        dialogueComponentNotificationHandler = nil,
        availabilityRequestBusHandler = nil,
    }

    self.__index = self
    return setmetatable(o, self)
end

function ScriptDialogueComponent:IsDebugEnabled()
    return self.Properties.EnableDebug
end

function ScriptDialogueComponent:LogError(title, msg)
    Debug.Log("Error: [dialogue_component.lua] " .. title .. " - " .. msg)
end

function ScriptDialogueComponent:LogInfo(title, msg)
    Debug.Log("Info: [dialogue_component.lua] " .. title .. " - " .. msg)
end

-- Performs the basic setup needed to establish communication between us and the DialogueComponent on the entity we're attached to.
function ScriptDialogueComponent:ActivateConversationScript()
    assert(self.InitConversationScript ~= nil,
        "Missing required InitConversationScript function!")

    if self.InitConversationScript ~= nil then
        self:InitConversationScript()
    end

    self.dialogueComponentNotificationHandler = DialogueComponentNotificationBus.Connect(self, self.entityId)
    self.availabilityRequestBusHandler = AvailabilityRequestBus.Connect(self, self.entityId)
    self.companionScriptRequestHandler = DialogueScriptRequestBus.Connect(self, self.entityId)
end

function ScriptDialogueComponent:DeactivateConversationScript()
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

    self.Participants = nil
end

function ScriptDialogueComponent:OnConversationStarted(initiatingEntityId)
    self.Participants = { player = initiatingEntityId }
end

function ScriptDialogueComponent:OnConversationEnded()
end

function ScriptDialogueComponent:OnConversationAborted()
end

function ScriptDialogueComponent:GetOwnerEntityId()
    return self.entityId
end

function ScriptDialogueComponent:OnDialogue(dialogue, availableResponses)
    Debug.Log("DIALOGUE COMPONENT ON DIALOGUE")
end

-- @brief Executes the script attached to a dialogue.
--
-- Nothing happens if no script is found that matches the Id..
--
-- @param nodeId The Id of the node whose script needs to be run
function ScriptDialogueComponent:RunDialogueScript(nodeId)
    Debug.Log("RunDialogueScript - Node: " .. Name.ToString(nodeId))
    local node = self[Name.ToString(nodeId)]

    if type(node) ~= "table" then
        self:LogError("RunDialogueScript", "Node '" .. Name.ToString(nodeId) .. "' not found")
        return
    end

    if type(node.Script) == "function" then
        if self:IsDebugEnabled() then
            self:LogInfo("Ran dialogue script", Name.ToString(nodeId))
        end

        if self:IsDebugEnabled() then
            self:LogInfo("RunDialogueScript", "Running script for node '" .. Name.ToString(nodeId) .. "'")
        end

        node.Script({ speakerEntity = GetEntityByTag(Crc32()) })
        return
    end

    if (self:IsDebugEnabled()) then
        self:LogInfo("Unable to run dialogue script", "script not found for node '"
            .. Name.ToString(nodeId) .. "'")
    end
end

-------------------------------------------------------------------------------
-- @brief Runs and returns the return of the given node's condition script(s).
--
-- @note If no node is found, we assume there are no conditions and returning
--       true.
-- @returns true if all conditions are satisfied. Otherwise, returns false.
-------------------------------------------------------------------------------
function ScriptDialogueComponent:IsAvailable(nodeId)
    local node = self[nodeId]

    if node == nil then
        -- nil condition implies true 
        return true
    end

    if type(node) ~= "table" then
        return false
    end

    if (type(node.Condition) ~= "function") then
        Debug.Log(
            "[dialogue_component.lua] Condition is not a function as expected")
    end

    local result = node.Condition({ owner = self.entityId })
    if type(result) ~= "boolean" then
        self:LogError("Unable to check availability",
            "a boolean was not returned as expected")
        return false
    end

    return result
end

-------------------------------------------------------------------------------
-- @brief Helper for adding a condition script to a dialogue.
--
-- @param dialogueNodeName The name of the node to add the condition to.
-- @param conditionFunction A function returning a boolean result.
-------------------------------------------------------------------------------
function ScriptDialogueComponent:AddCondition(dialogueNodeName,
                                              conditionFunction)
    local e = "Unable to add condition"
    if type(dialogueNodeName) ~= "string" then
        self:LogError(e, "received non-string node name")
        return
    end

    if not (string.len(dialogueNodeName) > 0) then
        self:LogError(e, "empty node name")
    end

    if conditionFunction == nil then
        return
    end

    -- Currently, we only accept function types
    if type(conditionFunction) == "function" then
        self.conditions[dialogueNodeName] = conditionFunction
        if self:IsDebugEnabled() then
            self:LogInfo("Condition added", "node '" ..
                dialogueNodeName "'")
        end
        return
    end

    if self:IsDebugEnabled() then
        self:LogInfo(
            e, "a valid condition function was not provided")
    end

end

return ScriptDialogueComponent
