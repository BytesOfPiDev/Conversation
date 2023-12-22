-- ConversationGraphName script component

require("conversation.core")
local dcLib = require("conversation.dialogue_component")

-- BOP_GENERATED_INCLUDES_BEGIN

-- BOP_GENERATED_INCLUDES_END

-- BOP_GENERATED_FUNCTIONS_BEGIN

-- BOP_GENERATED_FUNCTIONS_END

local ConversationGraphName = dcLib.ScriptDialogueComponent:New()

function ConversationGraphName:InitConversationScript()
	-- BOP_GENERATED_INSTRUCTIONS_BEGIN: in_branch1, in_branch2, in_branch3, in_branch4

	-- BOP_GENERATED_INSTRUCTIONS_END
end

function ConversationGraphName:OnActivate()
	self:ActivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.

	Debug.Log(
		"[ConversationGraphName] ScriptComponent activated on entity: '"
			.. GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)
			.. "'"
	)
end

function ConversationGraphName:OnDeactivate()
	self:DeactivateConversationScript() -- Required since defining OnActivate in the metaclass doesn't seem to work.

	Debug.Log(
		"[ConversationGraphName] ScriptComponent deactivated on entity: '"
			.. GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)
			.. "'"
	)
end

-- BOP_GENERATED_CONDITION_FUNCTIONS_BEGIN

-- BOP_GENERATED_CONDITION_FUNCTIONS_END

return ConversationGraphName
