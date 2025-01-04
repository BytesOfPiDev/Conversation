-- dynamic_layout.lua

local M = {}

function M.SetNumChildElements(entityId, num)
	UiDynamicLayoutBus.Event.SetNumChildElements(entityId, num)
end

function M:GetNumChildElements(entityId)
	return UiDynamicLayoutBus.Event.SetNumChildElements(entityId)
end

return M
