-- core.lua

require("conversation.dialogue_component")

function GetEntityByTag(tag)
	return TagGlobalRequestBus.Event.GetEntityByTag(Crc32(tag))
end

function LogConditionError(graphName, nodeName)
	Debug.Log(
		"ConditionError in graph '" .. graphName .. "', "
			.. "node '" .. nodeName .. "':  "
			.. "condition is not a function"
	)
end

Debug.Log("Conversation's 'core.lua' loaded.\n")
