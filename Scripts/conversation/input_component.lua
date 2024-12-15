-- dialogue_component_script.lua

local InputComponentBase = {
	available_responses = {},
	filtered_responses = {},
}

function InputComponentBase:New()
	local o = {
		Properties = {
			EnableDebug = { default = false, description = "Enable debug logging" },
		},
	}

	self.__index = self

	return setmetatable(o, self)
end

function InputComponentBase:IsDebugEnable()
	return self.Properties.EnableDebug
end

function InputComponentBase:LogError(title, msg)
	Debug.Log("Error: [dialogue_component.lua] " .. title .. " - " .. msg)
end

function InputComponentBase:LogInfo(title, msg)
	Debug.Log("Info: [dialogue_component.lua] " .. title .. " - " .. msg)
end

function InputComponentBase:Init()
	if self.Properties.EnableDebug then
		Debug.Log(self.script_name .. "OnActivate")
	end

	if DialogueComponentNotificationBus then
		self.dialogue_component_notification_bus_handler = DialogueComponentNotificationBus.Connect(self, self.entityId)
	end
end

function InputComponentBase:Shutdown()
	if self.Properties.EnableDebug then
		Debug.Log(self.script_name .. "OnDeactivate")
	end

	if DialogueComponentNotificationBus then
		self.dialogue_component_notification_bus_handler:Disconnect()
	end
end

function InputComponentBase:SelectAvailableResponse(responseIndex)
	if type(responseIndex) ~= "number" then
		LogError("SelectAvailableResponse", "Expected number, got " .. type(responseIndex))
	end

	LogInfo("SelectAvailableResponse", "Selecting response at index: " .. tostring(responseIndex))
	DialogueComponentBus.Event.SelectAvailableResponse(self.entityId, responseIndex)
end

function InputComponentBase:OnResponseAvailable(response)
	table.insert(self.available_responses, response)
end

return InputComponentBase
