-- ConversationGraphName script component

require("conversation.core")

-- BOP_GENERATED_INCLUDES_BEGIN

-- BOP_GENERATED_INCLUDES_END

local ConversationScriptComponent = ScriptDialogueComponent:new(nil, "Brah", "Tato")

function ConversationScriptComponent:Configure()
	-- BOP_GENERATED_INSTRUCTIONS_BEGIN: in_branch1, in_branch2, in_branch3, in_branch4

	-- BOP_GENERATED_INSTRUCTIONS_END
	for key, value in pairs(self.scripts) do
		self.scripts[key] = DialogueScript:new(nil, self.Properties.FirstName, nil)
	end
end

function ConversationScriptComponent:OnActivate()
	self:Setup(); -- Required since defining OnActivate in the metaclass doesn't seem to work.
end

function ConversationScriptComponent:OnDeactivate()
	self:Shutdown(); -- Required since defining OnActivate in the metaclass doesn't seem to work.
end

-- BOP_GENERATED_FUNCTIONS_BEGIN

-- BOP_GENERATED_FUNCTIONS_END

return ConversationScriptComponent
