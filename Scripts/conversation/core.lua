-- core.lua

require("conversation.condition")
require("conversation.dialogue")
require("conversation.dialogue_component")

local lib = {}

function lib:GetEntityByTag(tag)
	return TagGlobalRequestBus.Event.GetEntityByTag(Crc32(tag))
end

Debug.Log("Convesation's 'core.lua' loaded.\n")

return lib
