#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzFramework/Asset/GenericAssetHandler.h>
#include <Conversation/DialogueData.h>

namespace Conversation
{
	/** A string representing a specific script. */
	using DialogueScriptId = AZStd::string;

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
