#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/std/ranges/transform_view.h"
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

        static constexpr auto ProductExtension = "conversationasset";
        static constexpr auto ProductExtensionPattern = "*.conversationasset";
        static constexpr auto ProductDotExtension = ".conversationasset";

        static constexpr auto SourceExtension = "conversation";
        static constexpr auto SourceExtensionPattern = "*.conversation";
        static constexpr auto SourceDotExtension = ".conversation";

        static constexpr auto ProductAssetSubId = 1;

        [[nodiscard]] auto CountStartingIds() const -> size_t override
        {
            return m_startingIds.size();
        }

        [[nodiscard]] auto CountDialogues() const -> size_t override
        {
            return m_dialogues.size();
        }

        [[nodiscard]] auto CopyStartingIds() const -> AZStd::vector<UniqueId> override
        {
            return m_startingIds;
        }

        [[nodiscard]] auto CopyDialogues() const -> DialogueDataContainer override
        {
            return m_dialogues;
        }

        void AddStartingId(UniqueId const& newStartingId) override;
        void AddDialogue(DialogueData const& newDialogueData) override;
        /**
         * @brief Add a dialogue as a response to another dialogue
         *
         * As long as the ResponseData is valid (not null), the response will
         * be added, even if there is not currently a DialogueData in this
         * asset with matching DialogueIds.
         *
         * @param responseData The response to add.
         *
         * @note Not currently implemented.
         */

        void AddResponse(ResponseData const& responseData) override;

        auto GetDialogueById(UniqueId const& dialogueId) -> AZ::Outcome<DialogueData> override;

        [[nodiscard]] auto CheckDialogueExists(UniqueId const& dialogueId) -> bool override
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

        [[nodiscard]] auto GetMainScript() const -> AZ::Data::Asset<ConversationAsset>
        {
            return m_mainScript;
        }

        void SetMainScript(AZ::Data::Asset<ConversationAsset> const& asset)
        {
            m_mainScript = asset;
        }

        auto AddNames(AZStd::span<AZ::Name> names)
        {
            // TODO: Improve; maybe ranges, views, transform
            AZStd::ranges::for_each(
                names,
                [this](auto const& name)
                {
                    m_names.insert(name);
                });
        }

    private:
        //! The IDs of any dialogues that can be used to begin a conversation.
        AZStd::vector<UniqueId> m_startingIds{};
        AZStd::vector<ResponseData> m_responses;
        AZStd::unordered_set<DialogueChunk> m_chunks;
        DialogueDataContainer m_dialogues{};
        AZStd::string m_comment{};
        AZ::Data::Asset<AZ::ScriptAsset> m_mainScript{};
        AZStd::unordered_set<AZ::Name> m_names{};
    };

    using ConversationAssetHandler = AzFramework::GenericAssetHandler<ConversationAsset>;

    using ConversationAssetContainer = AZStd::vector<AZ::Data::Asset<ConversationAsset>>;

    inline constexpr void AddStartingId(ConversationAsset& conversationAsset, UniqueId const& startingId)
    {
        conversationAsset.AddStartingId(startingId);
    }
} // namespace Conversation
