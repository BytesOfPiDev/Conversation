#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Outcome/Outcome.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/std/containers/unordered_set.h"
#include "AzCore/std/containers/vector.h"
#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{

    struct DialogueData;
    struct DialogueId;
    struct ResponseData;
    struct DialogueChunk;

    /*
     * The interface that objects that manage ConversationAsset or ConversationAsset-like objects implement.
     *
     * TODO: There's a "Get" and "Copy" version of some functions. I'm not entirely sure how ScriptCanvas/Lua handle references, so I'm
     * distinguing them until I find out for sure.
     */
    class IConversationAsset
    {
    public:
        AZ_RTTI(IConversationAsset, ConversationAssetInterfaceTypeId); // NOLINT
        AZ_DISABLE_COPY_MOVE(IConversationAsset); // NOLINT

        IConversationAsset() = default;
        virtual ~IConversationAsset() = default;

        /*
         * @brief Returns the amount of DialogueIds that can potentially be used to start a conversation.
         */
        [[nodiscard]] virtual auto CountStartingIds() const -> size_t = 0;

        /*
         * @brief Returns the amount of DialogueData objects within the asset.
         */
        [[nodiscard]] virtual auto CountDialogues() const -> size_t = 0;

        /*
         * @brief Returns a reference to the DialogueIds that can potentially be used to start a conversation.
         */
        [[nodiscard]] virtual auto GetStartingIds() const -> AZStd::vector<DialogueId> const& = delete;
        /*
         * @brief Returns a copy of the DialogueIds that can potentially be used to start a conversation.
         */
        [[nodiscard]] virtual auto CopyStartingIds() const -> AZStd::vector<DialogueId> = 0;

        /*
         * @brief Returns a reference to the DialogueData objects within the asset.
         */
        [[nodiscard]] virtual auto GetDialogues() const -> AZStd::unordered_set<DialogueData> const& = delete;
        /*
         * @brief Returns a copy of the DialogueData objects within the asset.
         */
        [[nodiscard]] virtual auto CopyDialogues() const -> AZStd::unordered_set<DialogueData> = 0;

        virtual void AddStartingId(DialogueId const& newStartingId) = 0;
        virtual void AddDialogue(DialogueData const& newDialogueData) = 0;
        virtual void AddResponse(ResponseData const& responseData) = 0;

        [[nodiscard]] virtual auto GetDialogueById(DialogueId const& dialogueId) -> AZ::Outcome<DialogueData> = 0;
        [[nodiscard]] virtual auto CheckDialogueExists(DialogueId const& dialogueId) -> bool = 0;

        [[nodiscard]] virtual auto GetMainScriptAsset() const -> AZ::Data::Asset<AZ::ScriptAsset> = 0;

        virtual void AddChunk(DialogueChunk const& dialogueChunk) = 0;
    };
} // namespace Conversation
