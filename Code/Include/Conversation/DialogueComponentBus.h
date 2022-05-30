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

        virtual void TryToStartConversation([[maybe_unused]] const AZ::EntityId& initiatingEntityId)
        {
        }

        virtual void SelectDialogue([[maybe_unused]] const DialogueData& dialogueToSelect)
        {
        }

        virtual void AbortConversation()
        {
        }

        virtual void EndConversation()
        {
        }

        virtual bool CheckAvailability([[maybe_unused]] const DialogueData& dialogueToCheck)
        {
            return false;
        }

        virtual bool CheckAvailability([[maybe_unused]] const DialogueId& dialogueIdToCheck)
        {
            return false;
        }
    };
    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

    class DialogueComponentNotifications : public AZ::ComponentBus
    {
    public:
        virtual void OnDialogue(
            [[maybe_unused]] const DialogueData& dialogue, [[maybe_unused]] const AZStd::vector<DialogueData>& availableResponses)
        {
        }
        virtual void OnConversationStarted([[maybe_unused]] const AZ::EntityId initiatingEntityId)
        {
        }
        virtual void OnConversationAborted()
        {
        }
        virtual void OnConversationEnded()
        {
        }
    };

    using DialogueComponentNotificationBus = AZ::EBus<DialogueComponentNotifications>;

} // namespace Conversation
