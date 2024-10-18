#pragma once

#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfo.h"

namespace ConversationCanvas
{
    class LuaSnippet
    {
    public:
        AZ_TYPE_INFO_WITH_NAME(
            LuaSnippet, "LuaSnippet", "{39DCF143-3089-4955-A8A9-9EBFC3BA6C15}");

        static void Reflect(AZ::ReflectContext* context);

        [[nodiscard]] auto GetSourceCode() const -> AZStd::string_view
        {
            return m_code;
        }

        [[nodiscard]] auto SetValue(AZStd::string_view code)
        {
            m_code = code;
        }

    private:
        AZStd::string m_code{};
    };
} // namespace ConversationCanvas