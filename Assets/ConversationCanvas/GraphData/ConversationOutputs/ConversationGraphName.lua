-- ConversationGraphName script component

local dcLib = require("conversation.dialogue_component")

local ConversationScriptComponent = dcLib.ScriptDialogueComponent:New()

function ConversationScriptComponent:InitConversationScript()
	-- BOP_GENERATED_INSTRUCTIONS_BEGIN: in_branch1, in_branch2, in_branch3, in_branch4

	-- BOP_GENERATED_INSTRUCTIONS_END
end

function ConversationScriptComponent:OnActivate()
	self:ActivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.

	Debug.Log(
		"[ConversationGraphName] ScriptComponent activated on entity: '"
			.. GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)
			.. "'"
	)
end

function ConversationScriptComponent:OnDeactivate()
	self:DeactivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.

	Debug.Log(
		"[ConversationGraphName] ScriptComponent deactivated on entity: '"
			.. GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)
			.. "'"
	)
end

return ConversationScriptComponent
