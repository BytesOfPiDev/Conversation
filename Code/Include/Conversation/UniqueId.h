#pragma once

#include "AzCore/Math/Uuid.h"
#include "AzCore/RTTI/ReflectContext.h"

#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{

    constexpr auto InvalidDialogueId{ 0 };

    /**
     * Represents a locally unique identifier for various data types.
     *
     * @note Currently, it is primarily used with DialogueData instances, but it will be used for
     * any object instance that needs a unique ID.
     */
    struct UniqueId
    {
    public:
        AZ_TYPE_INFO(UniqueId, DialogueIdTypeId); // NOLINT

        friend void ReflectDialogueId(AZ::ReflectContext* context);

        static auto CreateInvalidId()
        {
            return UniqueId{};
        }

        static auto CreateNamedId(AZStd::string_view name)
        {
            UniqueId newDialogueId{};
            newDialogueId.m_id = AZ::Crc32(name);
            return newDialogueId;
        }

        static auto CreateRandomId()
        {
            return CreateNamedId(AZ::Uuid::CreateRandom().ToFixedString());
        }

        [[nodiscard]] constexpr auto IsValid() const -> bool
        {
            return m_id != InvalidDialogueId;
        }

        [[nodiscard]] auto operator==(UniqueId const& other) const -> bool
        {
            return m_id == other.m_id;
        }

        [[nodiscard]] auto operator!=(UniqueId const& other) const -> bool
        {
            return m_id != other.m_id;
        }

        [[nodiscard]] auto GetHash() const -> size_t
        {
            return m_id;
        }

    private:
        size_t m_id;
    };

    static_assert(AZStd::is_pod_v<UniqueId>, "Ensure UniqueId is POD");

} // namespace Conversation
