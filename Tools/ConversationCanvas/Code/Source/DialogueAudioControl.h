#pragma once

#include "AzCore/RTTI/ReflectContext.h"

namespace Conversation
{
    class DialogueAudioControl
    {
    public:
        AZ_TYPE_INFO_WITH_NAME(
            DialogueAudioControl,
            "ConversationControl",
            "{839DC3CD-8F3B-43B3-AE9D-3EC3A4133ADA}");

        static void Reflect(AZ::ReflectContext* context);

        constexpr auto GetName() const -> AZStd::string_view
        {
            return m_control;
        }

        auto IsEmpty() const
        {
            return m_control.empty();
        }

    private:
        AZStd::string m_control{};
    };
} // namespace Conversation
