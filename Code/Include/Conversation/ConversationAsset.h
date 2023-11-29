#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzFramework/Asset/GenericAssetHandler.h"
#include "Conversation/DialogueData.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    class ConversationAsset
        : public AZ::Data::AssetData
        , public IConversationAsset
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(ConversationAsset); // NOLINT
        AZ_RTTI_NO_TYPE_INFO_DECL(); // NOLINT
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationAsset); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        ConversationAsset() = default;
        ~ConversationAsset() override = default;

        static constexpr const char* ProductExtension = "conversationasset";
        static constexpr const char* ProductExtensionPattern = "*.conversationasset";

        static constexpr const char* SourceExtension = "conversation";
        static constexpr const char* SourceExtensionPattern = "*.conversation";

        [[nodiscard]] auto CountStartingIds() const -> size_t override
        {
            return m_startingIds.size();
        }

        [[nodiscard]] auto CountDialogues() const -> size_t override
        {
            return m_dialogues.size();
        }

        [[nodiscard]] auto GetStartingIds() const -> const AZStd::vector<DialogueId>& override
        {
            return m_startingIds;
        }

        [[nodiscard]] auto GetDialogues() const -> const DialogueDataContainer& override
        {
            return m_dialogues;
        }

        void AddStartingId(const DialogueId& newStartingId) override;
        void AddDialogue(const DialogueData& newDialogueData) override;
        /**
         * @brief Add a dialogue as a response to another dialogue
         *
         * As long as the ResponseData is valid (not null), the response will
         * be added, even if there is not currently a DialogueData in this
         * asset with matching DialogueIds.
         *
         * @param responseData The response to add.
         */

        void AddResponse(ResponseData const& responseData) override;
        auto GetDialogueById(const DialogueId& dialogueId) -> AZ::Outcome<DialogueData> override;
        [[nodiscard]] auto CheckDialogueExists(const DialogueId& dialogueId) -> bool override
        {
            return m_dialogues.contains(DialogueData(dialogueId));
        }

        [[nodiscard]] auto GetMainScriptAsset() const -> AZ::Data::Asset<AZ::ScriptAsset> override
        {
            return m_mainScript;
        }

        void AddChunk(DialogueChunk const& dialogueChunk) override
        {
            m_chunks.insert(dialogueChunk);
        }

        [[nodiscard]] auto GetResponses() const -> AZStd::vector<ResponseData> const& override
        {
            return m_responses;
        }

    private:
        //! The IDs of any dialogues that can be used to begin a conversation.
        AZStd::vector<DialogueId> m_startingIds{};
        AZStd::vector<ResponseData> m_responses;
        AZStd::unordered_set<DialogueChunk> m_chunks;
        DialogueDataContainer m_dialogues{};
        AZStd::string m_comment{};
        AZ::Data::Asset<AZ::ScriptAsset> m_mainScript{};
    };

    using ConversationAssetHandler = AzFramework::GenericAssetHandler<ConversationAsset>;

    using ConversationAssetContainer = AZStd::vector<AZ::Data::Asset<ConversationAsset>>;

    inline constexpr void AddStartingId(ConversationAsset& conversationAsset, DialogueId const& startingId)
    {
        conversationAsset.AddStartingId(startingId);
    }
} // namespace Conversation
