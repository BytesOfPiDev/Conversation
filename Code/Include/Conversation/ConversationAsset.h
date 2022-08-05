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

        using Pointer = AZ::Data::Asset<ConversationAsset>;
        static constexpr const char* PRODUCT_EXTENSION_PATTERN = "*.conversation";
        static constexpr const char* SOURCE_EXTENSION_PATTERN = "*.conversationdoc";

        static constexpr const char* PRODUCT_EXTENSION = "conversation";
        static constexpr const char* SOURCE_EXTENSION = "conversationdoc";

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

        const DialogueIdUnorderedSetContainer& GetStartingIds() const
        {
            return m_startingIds;
        }

        const DialogueDataUnorderedSetContainer& GetDialogues() const
        {
            return m_dialogues;
        }

        void AddStartingId(const DialogueId& newStartingId);
        void AddDialogue(const DialogueData& newDialogueData);
        void AddResponseId(const DialogueId& parentDialogueId, const DialogueId& responseDialogueId);
        AZ::Outcome<DialogueData> GetDialogueById(const DialogueId& dialogueId);
        bool CheckDialogueExists(const DialogueId& dialogueId)
        {
            return m_dialogues.contains(DialogueData(dialogueId));
        }

    private:
        /**
         * The IDs of any dialogues that can be used to begin a conversation.
         */
        DialogueIdUnorderedSetContainer m_startingIds;
        DialogueDataUnorderedSetContainer m_dialogues;
    };

    using ConversationAssetHandler = AzFramework::GenericAssetHandler<ConversationAsset>;
} // namespace Conversation
