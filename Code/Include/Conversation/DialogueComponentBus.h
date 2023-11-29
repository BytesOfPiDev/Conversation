#pragma once

#include "AzCore/Component/ComponentBus.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    AZ_ENUM_CLASS( // NOLINT
        DialogueState,
        Invalid,
        Inactive,
        Starting,
        Active,
        Aborting,
        Ending);

    AZ_ENUM_CLASS( // NOLINT
        ConversationActiveSubState,
        Invalid,
        CheckingConditions,
        Executing,
        WaitingForResponse);

    class DialogueComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_DISABLE_COPY_MOVE(DialogueComponentRequests); // NOLINT

        DialogueComponentRequests() = default;
        ~DialogueComponentRequests() override = default;

    public: // Requests
        [[nodiscard]] virtual auto GetStartingIds() const -> AZStd::vector<DialogueId> const& = 0;
        [[nodiscard]] virtual auto GetDialogues() const -> AZStd::unordered_set<DialogueData> const& = 0;
        [[nodiscard]] virtual auto FindDialogue(const DialogueId& /*dialogueIdToFind*/) const -> DialogueData = 0;
        [[nodiscard]] virtual auto CheckIfDialogueIdExists(const DialogueId& /*dialogueId*/) const -> bool = 0;
        [[nodiscard]] virtual auto GetConversationAssets() const -> ConversationAssetContainer const& = 0;
        [[nodiscard]] virtual auto GetSpeakerTag() const -> AZStd::string = 0;

        virtual auto TryToStartConversation(const AZ::EntityId& /*initiatingEntityId*/) -> AZStd::optional<AZStd::string> = 0;
        /**
         * Sends out the given DialogueData, making it the active dialogue.
         *
         * \param dialogueToSelect The desired dialogue.
         *
         * \note There is nothing that enforces that this dialogue actually
         * exist in the assets attached to this component. Potentially, this
         * may allow injecting a dialogue if desired.
         */
        virtual void SelectDialogue(const DialogueData& /*dialogueToSelect*/) = 0;

        /**
         * Attempts to find and send out a DialogueData using its DialogueId.
         *
         * Does nothing if it could not find one.
         */
        virtual void SelectDialogue(const DialogueId dialogueIdToFindAndSelect) = 0;

        virtual void SelectAvailableResponse(int const availableResponseIndex) = 0;
        virtual void AbortConversation() = 0;
        virtual void EndConversation() = 0;
        virtual void ContinueConversation() = 0;

        [[nodiscard]] virtual auto CheckAvailability(const DialogueData& dialogueToCheck) -> bool = 0;
        [[nodiscard]] virtual auto CheckAvailability(const DialogueId& dialogueIdToCheck) -> bool = 0;
        [[nodiscard]] virtual auto GetDisplayName() const -> AZStd::string = 0;
        [[nodiscard]] virtual auto GetAvailableResponses() const -> AZStd::vector<DialogueData> = 0;
        [[nodiscard]] virtual auto GetActiveDialogue() const -> DialogueData = 0;
        [[nodiscard]] virtual auto GetCurrentState() const -> DialogueState = 0;
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
        virtual void OnResponseAvailable([[maybe_unused]] const DialogueData& availableDialogue)
        {
        }
    };

    using DialogueComponentNotificationBus = AZ::EBus<DialogueComponentNotifications>;

    class GlobalConversationNotifications : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(GlobalConversationNotifications); // NOLINT

        GlobalConversationNotifications() = default;
        virtual ~GlobalConversationNotifications() = default;
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
