#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Conversation/ConversationAsset.h>

namespace Conversation
{
    class ConversationAsset;

    using ConversationAssetContainer = AZStd::vector<AZ::Data::Asset<ConversationAsset>>;

    class DialogueComponentRequests : public AZ::ComponentBus
    {
    public:
        ~DialogueComponentRequests() override = default;

    public: // Requests

        virtual AZStd::unordered_set<DialogueId> GetStartingIds() const
        {
            return AZStd::unordered_set<DialogueId>();
        }

        virtual AZStd::unordered_set<DialogueData> GetDialogues() const
        {
            return AZStd::unordered_set<DialogueData>();
        }

        virtual DialogueData FindDialogue(const DialogueId& /*dialogueId*/) const
        {
            return DialogueData();
        }

        virtual bool CheckIfDialogueIdExists(const DialogueId& /*dialogueId*/) const
        {
            return false;
        }

        virtual const ConversationAssetContainer GetConversationAssets() const
        {
            return AZStd::move(ConversationAssetContainer());
        }

        virtual AZStd::string GetSpeakerTag() const
        {
            return {};
        }
    };

    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

} // namespace Conversation
