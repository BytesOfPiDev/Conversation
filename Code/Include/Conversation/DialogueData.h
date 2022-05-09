#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Outcome/Outcome.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/containers/set.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/string/string_view.h>
#include <AzFramework/Asset/GenericAssetHandler.h>

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    using DialogueId = AZ::Uuid;
    using DialogueIdUnorderedSetContainer = AZStd::unordered_set<DialogueId>;
    using AvailabilityId = AZStd::string;
    enum class DialogueActorType : int
    {
        Invalid,
        Npc,
        Player
    };

    /**
     * @brief Represents a line of dialogue in a conversation.
     *
     * This class is used for both NPC and Player lines of dialogue.
     */
    class DialogueData
    {
    public:
        AZ_TYPE_INFO(DialogueData, "{6BF81F0F-0013-4877-80EB-4DC579005DDE}");
        AZ_CLASS_ALLOCATOR(DialogueData, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);
        // There's a limited to the maximum amount of dialogue responses.
        // Attempting to go beyond this will likely result in it not being added.
        constexpr static size_t MAX_RESPONSES = 10;

        DialogueData(bool generateRandomId = false);
        DialogueData(
            const DialogueId id, const AZStd::string actorText, const AZStd::string speaker, const DialogueIdUnorderedSetContainer& responses);
        /**
         * Create a piece of dialogue with the given ID.
         *
         * This can be useful when you want to write the data at a later time
         * It is also useful for comparing against another DialogueData, as two
         * DialogueData's are considered the same if their ID's match.
         *
         * \param id
         */
        DialogueData(const DialogueId id);
        ~DialogueData() = default;

        bool operator==(const DialogueData& other) const
        {
            return m_id == other.m_id;
        }

        /**
         * Determine if this dialogue's ID is equal to the given dialogue ID.
         *
         * This is useful for keeping DialogueId's instead of entire DialogueData objects
         * and then subsequently using the ID to find a matching DialogueData.
         *
         * \param dialogueId The DialogueId to compare against.
         * \return Whether the DialogueId of this DialogueData matches the DialogueId given.
         */
        bool operator==(const DialogueId& dialogueId) const
        {
            return m_id == dialogueId;
        }

        bool operator<(const DialogueData& other) const
        {
            return m_id < other.m_id;
        }

        bool operator>(const DialogueData& other) const
        {
            return m_id > other.m_id;
        }

        bool operator<=(const DialogueData& other) const
        {
            return !(m_id > other.m_id);
        }

        bool operator>=(const DialogueData& other) const
        {
            return !(m_id < other.m_id);
        }

        bool IsValid() const
        {
            return !GetId().IsNull();
        }

        DialogueId GetId() const
        {
            return m_id;
        }
        AZStd::string GetActorText() const
        {
            return m_actorText;
        }

        AZStd::string GetSpeaker() const
        {
            return m_speaker;
        }
        const DialogueIdUnorderedSetContainer& GetResponseIds() const
        {
            return m_responseIds;
        }
        AZStd::string GetAudioTrigger() const
        {
            return m_audioTrigger;
        }
        AZStd::string GetComment() const
        {
            return m_comment;
        }

        void SetActorText(const AZStd::string& actorText)
        {
            m_actorText = actorText;
        }

        void SetSpeaker(const AZStd::string& speaker)
        {
            m_speaker = speaker;
        }
        /**
         * @brief Add a response ID that can follow this dialogue.
         *
         * The response ID should match an existing DialogueData located elsewhere
         * in the dialogue asset that houses this dialogue, but it doesn't have to.
         * If it doesn't, the dialogue system should be smart (or will be) smart
         * enough to just ignore it. This allows for potentially adding it later.
         *
         * There is a maximum amount of responses that can be added to a dialogue.
         * This limit is set in DialogueData::MAX_RESPONSES. Attempting to go beyond
         * this limit will likely result in the response not being added. Don't
         * do it.
         *
         * @param responseId The DialogueId of the potential reponse.
         */
        void AddResponseId(const DialogueId responseId)
        {
            // Only add the response if we're within the limit.
            if (m_responseIds.size() < MAX_RESPONSES)
            {
                m_responseIds.insert(responseId);
            }
        }
        void SetAudioTrigger(const AZStd::string& audioTrigger)
        {
            m_audioTrigger = audioTrigger;
        }

        void SetEntryDelay(float entryDelay)
        {
            m_entryDelay = entryDelay;
        }

        void SetComment(const AZStd::string& comment)
        {
            m_comment = comment;
        }

    private:
        DialogueId m_id;
        AZStd::string m_actorText = "";
        AZStd::string m_speaker;
        DialogueIdUnorderedSetContainer m_responseIds;
        AZStd::string m_audioTrigger;
        AZStd::string m_comment;
        float m_entryDelay = 5;
    };

    using DialogueDataPtr = AZStd::shared_ptr<DialogueData>;
    using DialogueDataUnorderedSetContainer = AZStd::unordered_set<DialogueData>;

} // namespace Conversation

namespace AZStd
{
    template<>
    struct hash<Conversation::DialogueData>
    {
        size_t operator()(const Conversation::DialogueData& obj) const
        {
            return obj.GetId().GetHash();
        }
    };
} // namespace AZStd
