-- Conversation component input script
local ConversationComponentInput = require("conversation.input_component").New()

function ConversationComponentInput:OnActivate() end

function ConversationComponentInput:OnDeactivate() end

function ConversationComponentInput:OnDialogue(dialogue_data)
	self.active_dialogue = nil
	self.active_dialogue = dialogue_data
end

function ConversationComponentInput:OnContinuePressed()
	local default_response_index <const> = 0
	self:SelectAvailableResponse(0)
end

function ConversationComponentInput:OnDialogueBegin() end

function ConversationComponentInput:OnDialogueEnd() end

return ConversationComponentInput
