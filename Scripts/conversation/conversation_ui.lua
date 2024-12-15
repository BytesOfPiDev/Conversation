-- ui.lua
local Ui = {
	Properties = {
		EnableDebug = {
			default = false,
			description = "Enable debug logging",
		},
		PlayerTextEntity = { default = EntityId(), description = "" },
		SpeakerTextEntity = { default = EntityId(), description = "" },
		ResponseLayout = { default = EntityId(), description = "" },
	},
	script_name = "conversation_ui.lua: ",
	active_dialogue_data = nil,
	available_responses = {},
}

local text_bus = require("conversation.ui.text")
local dynamic_layout = require("conversation.ui.dynamic_layout")

function Ui:LogError(title, msg)
	Debug.Log("ERROR: [" .. self.script_name .. "] " .. title .. " - " .. msg)
end

function Ui:LogInfo(title, msg)
	Debug.Log("INFO: [" .. self.script_name .. "]" .. title .. " - " .. msg)
end

function Ui:OnActivate()
	local fn = "OnActivate"

	if self.Properties.EnableDebug and not self.Properties.PlayerTextEntity then
		self:LogError(fn, "The player text entity is not set!")
	end

	if self.Properties.EnableDebug and not self.Properties.ResponseLayout then
		self:LogError(fn, "The response layout entity is not set!")
	end

	if self.Properties.EnableDebug and not self.Properties.SpeakerTextEntity then
		self:LogError(fn, "The speaker text entity is not set!")
	end

	if self.Properties.EnableDebug and not PlayerSpeakerTag then
		self:LogError(fn, "PlayerSpeakerTag value: " .. PlayerSpeakerTag)
	else
		self.player_entity = TagGlobalRequestBus.Broadcast.GetEntityByTag(PlayerSpeakerTag)
		self.dialogue_component_notification_bus_handler = DialogueComponentNotificationBus.Connect(self, self.entityId)
	end

	text_bus.SetText(self.Properties.PlayerTextEntity, "...")
	text_bus.SetText(self.Properties.SpeakerTextEntity, "...")
	dynamic_layout.SetNumChildElements(self.Properties.ResponseLayout, 8)
end

function Ui:OnDeactivate()
	if self.Properties.EnableDebug then
		Debug.Log(self.script_name .. "OnDeactivate")
	end

	if DialogueComponentNotificationBus then
		self.dialogue_component_notification_bus_handler:Disconnect()
	end
end

function Ui:OnDialogue(dialogue_data)
	self.active_dialogue_data = nil
	self.active_dialogue_data = dialogue_data
end

function Ui:OnDialogueBegin() end

function Ui:OnDialogueEnd() end

function Ui:OnResponseAvailable(response)
	table.insert(self.available_responses, response)
end

return Ui
