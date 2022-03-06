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
            const DialogueActorType actorType,
            const DialogueId id,
            const AZStd::string actorText,
            const AZStd::string availabilityscript,
            const AZStd::string dialogueScript,
            const AZStd::string speaker,
            const AZStd::set<DialogueId>& responses);
        ~DialogueData() = default;

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
        AZStd::string GetAvailabilityScript() const
        {
            return m_availabilityScript;
        }
        AZStd::string GetScript() const
        {
            return m_script;
        }
        AZStd::string GetSpeaker() const
        {
            return m_speaker;
        }
        const AZStd::set<DialogueId>& GetResponseIds() const
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
        void SetAvailabilityScript(const AZStd::string& availabilityScript)
        {
            m_availabilityScript = availabilityScript;
        }
        void SetScript(const AZStd::string& script)
        {
            m_script = script;
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
        DialogueActorType m_actorType = DialogueActorType::Invalid;
        DialogueId m_id;
        AZStd::string m_actorText = "";
        AvailabilityId m_availabilityScript;
        AZStd::string m_script;
        AZStd::string m_speaker;
        AZStd::set<DialogueId> m_responseIds;
        AZStd::string m_audioTrigger;
        AZStd::string m_comment;
        float m_entryDelay = 5;
    };

    using DialogueDataPtr = AZStd::shared_ptr<DialogueData>;

    class ConversationAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI(ConversationAsset, "{C2B4E407-B74E-4E48-8B8A-ADD5BCC894D1}", AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(ConversationAsset, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        ConversationAsset() = default;
        ~ConversationAsset() override = default;

        size_t CountStartingIds() const
        {
            return m_startingIds.size();
        }

        size_t CountDialogues() const
        {
            return m_dialogues.size();
        }

        const AZStd::set<DialogueId>& GetStartingIds() const
        {
            return m_startingIds;
        }

        void AddStartingId(const DialogueId& newStartingId);
        void AddDialogue(const DialogueData& newDialogueData);
        void AddResponseId(const DialogueId& parentDialogueId, const DialogueId& responseDialogueId);
        AZ::Outcome<DialogueData> GetDialogueById(const DialogueId& dialogueId);
        bool CheckDialogueExists(const DialogueId& dialogueId)
        {
            return m_dialogues.contains(dialogueId);
        }

    private:
        /**
         * The IDs of any dialogues that can be used to begin a conversation.
         */
        AZStd::set<DialogueId> m_startingIds;
        /**
         * A map that associates each dialogue with their IDs.
         *
         * I am using a map for now, but I feel there's a better option.
         * The DialogueId is already stored inside the map - maybe I can
         * use operator overloading so dialogues can be compared for
         * equality solely based on their IDs, then I could use a set.
         */
        AZStd::unordered_map<DialogueId, DialogueData> m_dialogues;
    };

    using ConversationAssetHandler = AzFramework::GenericAssetHandler<ConversationAsset>;

} // namespace Conversation
