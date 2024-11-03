-- core.lua

require("conversation.dialogue_component")

local player_tag <const> = Crc32("player")
local conversation_camera_target_tag = Crc32("conversation_camera")

function GetEntityByTag(tag)
    if tag == nil then
        return EntityId()
    end

    local entityId = TagGlobalRequestBus.Event.GetEntityByTag(tag)
    if entityId == nil then
        return EntityId()
    end

    return entityId
end

function GetConversationCameraTargetTag()
    return conversation_camera_target_tag
end

function GetEntityByTagName(tagString)
    return TagGlobalRequestBus.Event.GetEntityByTag(tagString)
end

function GetPlayerEntityId()
    return TagGlobalRequestBus.Event.GetEntityByTag(player_tag)
end

function LogConditionError(graphName, nodeName)
    Debug.Log(
        "ConditionError in graph '" .. graphName .. "', "
            .. "node '" .. nodeName .. "':  "
            .. "condition is not a function"
    )
end

Debug.Log("Conversation's 'core.lua' loaded.\n")