-- ConversationGraphName_conditionScript.lua
-- This is a generated file. You shouldn't edit it manually, as it will be overwritten.
require("conversation.core")

local conditionFunction = function()
    local result = false;
    -- It is expected that the ConversationCanvas app will fill in the generated instructions below.
    -- BOP_GENERATED_INSTRUCTIONS_BEGIN: outValue

    -- result needs to be assigned to a boolean expression.

    -- BOP_GENERATED_INSTRUCTIONS_END

    if (result ~= true or result ~= false) then
        Debug.Log("Unexpected result from a conversation condition script function. It should be true or false.")
        return result;
    end
end

return conditionFunction;
