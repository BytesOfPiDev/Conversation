#pragma once

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Outcome/Outcome.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/std/containers/unordered_set.h"
#include "AzCore/std/containers/vector.h"

namespace Conversation
{

    struct DialogueData;
    struct DialogueId;
    struct ResponseData;
    struct DialogueChunk;

    class IConversationAsset
    {
    public:
        AZ_RTTI(IConversationAsset, "E055BA9A-31A0-48B5-B5B8-CD758771B151"); // NOLINT
        AZ_DEFAULT_COPY_MOVE(IConversationAsset); // NOLINT

        IConversationAsset() = default;
        virtual ~IConversationAsset() = default;

        [[nodiscard]] virtual auto CountStartingIds() const -> size_t = 0;

        [[nodiscard]] virtual auto CountDialogues() const -> size_t = 0;

        [[nodiscard]] virtual auto GetStartingIds() const -> AZStd::vector<DialogueId> const& = 0;

        [[nodiscard]] virtual auto GetDialogues() const -> AZStd::unordered_set<DialogueData> const& = 0;

        virtual void AddStartingId(DialogueId const& newStartingId) = 0;
        virtual void AddDialogue(DialogueData const& newDialogueData) = 0;
        virtual void AddResponse(ResponseData const& responseData) = 0;
        virtual auto GetDialogueById(const DialogueId& dialogueId) -> AZ::Outcome<DialogueData> = 0;
        [[nodiscard]] virtual auto CheckDialogueExists(const DialogueId& dialogueId) -> bool = 0;

        [[nodiscard]] virtual auto GetMainScriptAsset() const -> AZ::Data::Asset<AZ::ScriptAsset> = 0;

        virtual void AddChunk(DialogueChunk const& dialogueChunk) = 0;

        [[nodiscard]] virtual auto GetResponses() const -> AZStd::vector<ResponseData> const& = 0;
    };
} // namespace Conversation
