-- text.lua

local M = {}

function M.SetText(entityId, value)
	UiTextBus.Event.SetText(entityId, value)
end

return M
