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

        virtual DialogueData FindDialogue([[maybe_unused]] const DialogueId& dialogueIdToFind) const
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

        /**
         * Sends out the given DialogueData, making it the active dialogue.
         *
         * \param dialogueToSelect The desired dialogue.
         * 
         * \note There is nothing that enforces that this dialogue actually
         * exist in the assets attached to this component. Potentially, this
         * may allow injecting a dialogue if desired.
         */
        virtual void SelectDialogue([[maybe_unused]] const DialogueData& dialogueToSelect)
        {
        }

        /**
        * Attempts to find and send out a DialogueData using its DialogueId.
        * 
        * Does nothing if it could not find one.
        */
        virtual void SelectDialogue([[maybe_unused]] const DialogueId dialogueIdToFindAndSelect)
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

        virtual void ContinueConversation()
        {
        }
    };
    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

    class DialogueComponentNotifications : public AZ::ComponentBus
    {
    public:
        /**
        * Sent out when a dialogue is selected/spoken.
        * 
        * @param dialogue The dialogue that was sent out and is now active.
        * @param potentialResponses (Pending removal) A list of responses that may be sent out. 
        */
        virtual void OnDialogue(
            [[maybe_unused]] const DialogueData& dialogue, [[maybe_unused]] const AZStd::vector<DialogueData>& potentialResponses)
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
        /**
        * A dialogue choice that can be selected.
        * 
        * This notification may be sent zero or multiple times after a dialogue has
        * been sent out. 
        * 
        * @note A response can be for any entity in the game - both the player and NPCs.
        */
        virtual void OnResponseAvailable([[maybe_unused]]const DialogueData& availableDialogue)
        {
        }
    };

    using DialogueComponentNotificationBus = AZ::EBus<DialogueComponentNotifications>;

    class GlobalConversationNotifications : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        virtual void OnConversationStarted(AZ::EntityId /*initiator*/, AZ::EntityId /*target*/)
        {
        }

        virtual void OnConversationAborted(AZ::EntityId /*target*/)
        {
        }

        virtual void OnConversationEnded(AZ::EntityId /*target*/)
        {
        }

        virtual void OnWaitingForResponse([[maybe_unused]] AZ::EntityId target)
        {
        }
    };

    using GlobalConversationNotificationBus = AZ::EBus<GlobalConversationNotifications>;

} // namespace Conversation
