#pragma once

#include "Conversation/UniqueId.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    /**
     * Represents the data needed to create a response.
     **/
    struct ResponseData
    {
        AZ_TYPE_INFO(ResponseData, Conversation::ResponseDataTypeId); // NOLINT
        static void Reflect(AZ::ReflectContext* context);

        [[nodiscard]] auto IsValid() const -> bool
        {
            return m_parentDialogueId.IsValid() &&
                m_responseDialogueId.IsValid();
        }

        // The Id of the dialogue to create a response to.
        UniqueId m_parentDialogueId;
        // The Id of the dialogue that will become a response.
        UniqueId m_responseDialogueId;
    };

    static_assert(AZStd::is_pod_v<ResponseData>, "Ensure ResponseData is POD");

} // namespace Conversation
