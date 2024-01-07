#pragma once

#include "AzCore/std/string/string.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    /**
     * @brief Type for holding simple, as well as advanced dialogue text.
     *
     * Once fully implemented, it will enable customization of text based on
     * information available at the time of execution.
     */
    struct DialogueChunk
    {
    public:
        AZ_TYPE_INFO( // NOLINT(modernize-use-trailing-return-type)
            DialogueChunk,
            "{81C04F0E-BA10-4D66-A3E8-0304FB8EB545}");

        friend void ReflectDialogueChunk(AZ::ReflectContext* reflect);

        [[nodiscard]] auto operator==(DialogueChunk const& rhs) const -> bool
        {
            return GetHash() == rhs.GetHash();
        }

        void SetData(AZStd::string_view data)
        {
            m_data = data;
        }

        [[nodiscard]] auto GetData() const -> AZStd::string_view
        {
            return m_data;
        }

        [[nodiscard]] auto GetHash() const -> size_t
        {
            constexpr AZStd::hash<AZStd::string> hasher;
            return hasher(m_data);
        }

    private:
        AZStd::string m_data;
    };

} // namespace Conversation
