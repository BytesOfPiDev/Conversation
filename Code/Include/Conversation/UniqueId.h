#pragma once

#include "AzCore/Math/Uuid.h"
#include "AzCore/RTTI/ReflectContext.h"

#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{

    constexpr AZ::Name::Hash InvalidDialogueId{};

    /**
     * Represents a locally unique identifier for various data types.
     *
     * @note Currently, it is primarily used with DialogueData instances, but it
     * will be used for any object instance that needs a unique ID.
     */
    struct UniqueId
    {
    public:
        AZ_TYPE_INFO(UniqueId, DialogueIdTypeId); // NOLINT

        friend void ReflectUniqueId(AZ::ReflectContext* context);

        static auto CreateInvalidId()
        {
            return UniqueId{};
        }

        static auto CreateNamedId(AZStd::string_view name)
        {
            UniqueId newDialogueId{};
            newDialogueId.m_value = AZ::Name{ name }.GetHash();

            return newDialogueId;
        }

        static auto CreateRandomId()
        {
            return CreateNamedId(AZ::Uuid::CreateRandom().ToFixedString());
        }

        [[nodiscard]] constexpr auto IsValid() const -> bool
        {
            return m_value != InvalidDialogueId;
        }

        [[nodiscard]] auto operator==(UniqueId const& other) const -> bool
        {
            return m_value == other.m_value;
        }

        [[nodiscard]] auto operator!=(UniqueId const& other) const -> bool
        {
            return m_value != other.m_value;
        }

        [[nodiscard]] auto GetHash() const -> AZ::Name::Hash
        {
            return m_value;
        }

    private:
        static constexpr auto MaxNameSize{ 64 };

        AZ::Name::Hash m_value;
    };

    static_assert(AZStd::is_pod_v<UniqueId>, "Ensure UniqueId is POD");

} // namespace Conversation
