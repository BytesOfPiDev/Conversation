-- ConversationGraphName script component

require("conversation.core")

local ConversationScriptComponent = ScriptDialogueComponent:new(nil)

function ConversationScriptComponent:InitConversationScript()
	-- BOP_GENERATED_INSTRUCTIONS_BEGIN: in_branch1, in_branch2, in_branch3, in_branch4

	-- BOP_GENERATED_INSTRUCTIONS_END
end

function ConversationScriptComponent:OnActivate()
	self:ActivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.
end

function ConversationScriptComponent:OnDeactivate()
	self:DeactivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.
end

return ConversationScriptComponent
