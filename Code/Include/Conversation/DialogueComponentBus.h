#pragma once

#include "AzCore/Component/ComponentBus.h"
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

    /**
     * Requests to an entity's DialogueComponent
     */
    class DialogueComponentRequests : public AZ::ComponentBus
    {
    public:
        AZ_DISABLE_COPY_MOVE(DialogueComponentRequests); // NOLINT

        DialogueComponentRequests() = default;
        ~DialogueComponentRequests() override = default;

    public:
        /**
         * Tries to start a conversation.
         *
         * @param initiatingEntityId The entity initiating the conversation.
         * @returns bool True if the conversation successfully started.
         *
         * Success requires that:
         *    * The conversation is in the Inactive state.
         *    * At minimum, one ConversationAssetRefComponent on this entity.
         *    * At minimum, one valid DialogueData within any asset ref.
         *    * At minimum, one valid starting UniqueId within any asset ref.
         *    * At minimum, one valid starting UniqueId must match a
         *      DialogueData in the asset ref.
         *    * At minimum, one DialogueData matching a starting UniqueId must
         *      pass its availability check.
         *
         * While trying to start the conversation, the DialogueState may change.
         * Initially, it enters the DialogueState::Starting state. If all
         * requirements are met, a starting dialogue is chosen and the
         * dialogue state becomes Active. Otherwise, the conversation is reset
         * to Inactive.
         *
         * @note Callable from Lua
         * @note Callable from ScriptCanvas
         */
        virtual auto TryToStartConversation(AZ::EntityId initiatingEntityId)
            -> bool = 0;

        /**
         * Sends out the given DialogueData, making it the active dialogue.
         *
         * @param dialogueToSelect The desired dialogue.
         *
         * @note There is nothing that enforces that this dialogue actually
         * exist in the assets attached to this component. Potentially, this
         * may allow injecting a dialogue if desired.
         *
         * @note It isn't a const reference because it is possible for the
         * referenced object to cease existence while processing the selection.
         * It could be stored in a container or object that gets reset or
         * deleted.
         */
        virtual void SelectDialogue(DialogueData dialogueToSelect) = 0;

        /**
         * @brief Attempts to find a select a dialogue with the given Uniqueid.
         *
         * Checks all assets available to the entity for a DialogueData that has
         * the given UniqueId. If it finds one, an availability check is
         * performed. If the check passes, then the dialogue is selected.
         * Otherwise, nothing happens.
         *
         * @param id The id of the desired DialogueData.
         * @returns @c true if successfully selected, @c false otherwise.
         * @see ConversationAssetRefComponent
         */
        virtual auto TryToSelectDialogue(UniqueId const id) -> bool = 0;

        /**
         * @brief Selects one of the active dialogue's available responses.
         *
         * Responses numbers are based on the value of FirstResponseNumber
         *
         * @param responseNumber The choice number to select.
         */

        virtual void SelectAvailableResponse(int const responseNumber) = 0;
        /**
         * @brief Forcibly ends the conversation.
         *
         * After ending the conversation, the abort scripts are called.
         *
         * @note Used in situations such as a monster attacking the player
         * during a conversation.
         *
         * @note Conversations that only end through this call or from selecting
         * an ending dialogue in a conversation graph.
         */
        virtual void AbortConversation() = 0;

        /**
         * @brief Moves the conversation forward, if possible.
         *
         * Examples:
         *  - There is only one available response *and* it is by the same
         *    speaker as the active dialogue.
         */
        virtual void ContinueConversation() = 0;

        [[nodiscard]] virtual auto CheckAvailability(
            DialogueData const& dialogueToCheck) const -> bool = 0;
        [[nodiscard]] virtual auto CheckAvailabilityById(
            UniqueId const& dialogueIdToCheck) const -> bool = 0;
        [[nodiscard]] virtual auto GetAvailableResponses() const
            -> AZStd::vector<DialogueData> = 0;
        [[nodiscard]] virtual auto GetActiveDialogue() const
            -> AZ::Outcome<DialogueData> = 0;
        [[nodiscard]] virtual auto GetCurrentState() const -> DialogueState = 0;
    };

    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

    AZ_ENUM(
        DialogueComponentNotificationPriority, (Default, 0), (Internal, -1));

    class DialogueComponentNotifications : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::MultipleAndOrdered;

        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;

        using BusIdType = AZ::EntityId;

        struct BusHandlerOrderCompare
        {
            auto operator()(
                DialogueComponentNotifications* left,
                DialogueComponentNotifications* right) const -> bool
            {
                return left->GetDialogueComponentNotificationOrder() <
                    right->GetDialogueComponentNotificationOrder();
            }
        };

        /*
         * @brief Returns the order of a handler during notifications.
         *
         * Lower numbers get notified before higher numbers.
         *
         * Users should most likely not need to set this to anything other than
         * Default. It is primarly for Gem use. Particularly, it is used to make
         * sure the companion scripts receive notification before the rest of
         * the system. If user's do set it, it should be set to zero or more.
         * Negative numbers are reserved for the Gem.
         *
         * @returns A number indiciating position during noitification.
         *
         * @note The function can't be const due to behavior handlers.
         */
        [[nodiscard]] virtual auto GetDialogueComponentNotificationOrder()
            -> int = 0;

        /**
         * Sent out when a dialogue is selected/spoken.
         *
         * @param dialogue The dialogue that was sent out and is now active.
         * @param potentialResponses (Pending removal) A list of responses that
         * may be sent out.
         */
        virtual void OnDialogue(
            [[maybe_unused]] DialogueData const& dialogue,
            [[maybe_unused]] AZStd::vector<DialogueData> const&
                potentialResponses)
        {
        }
        virtual void OnConversationStarted(
            [[maybe_unused]] const AZ::EntityId initiatingEntityId)
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
         * This notification may be sent zero or multiple times after a dialogue
         * has been sent out.
         *
         * @note A response can be for any entity in the game - both the player
         * and NPCs.
         */
        virtual void OnResponseAvailable(
            [[maybe_unused]] DialogueData const& availableDialogue)
        {
        }
    };

    using DialogueComponentNotificationBus =
        AZ::EBus<DialogueComponentNotifications>;

    class GlobalConversationNotifications : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(GlobalConversationNotifications); // NOLINT

        GlobalConversationNotifications() = default;
        virtual ~GlobalConversationNotifications() = default;
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        virtual void OnConversationStarted(
            AZ::EntityId /*initiator*/, AZ::EntityId /*target*/)
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

    using GlobalConversationNotificationBus =
        AZ::EBus<GlobalConversationNotifications>;

} // namespace Conversation
