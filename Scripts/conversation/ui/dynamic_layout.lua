-- dynamic_layout.lua

local M = {}

function M.SetNumChildElements(entityId, num)
	UiDynamicLayoutBus.Event.SetNumChildElements(entityId, num)
end

return M
