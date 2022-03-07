
#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <Conversation/DialogueData.h>

namespace Conversation
{
    constexpr static const char* const SPEAKERTAG_PLAYER = "player";
    constexpr static const char* const SPEAKERTAG_OWNER = "owner";

    class ConversationRequests
    {
    public:
        AZ_RTTI(ConversationRequests, "{7e8571f3-9429-4224-8046-e0dc8bd62e0c}");
        virtual ~ConversationRequests() = default;
        // Put your public methods here

        /**
         * @brief Start a conversation using an entity's dialogue component.
         *
         * A DialogueComponent is required to be on the entity;
         *
         * @param entityId The entity that contains the DialogueComponent to use.
         */
        virtual void StartConversation(const AZ::EntityId /*entityId*/)
        {
        }
        virtual void EndConversation()
        {
        }
        virtual void AbortConversation()
        {
        }

        virtual void SelectResponseByNumber([[maybe_unused]] const size_t choiceNumber)
        {
        }
        virtual void SelectResponseById(const DialogueId&)
        {
        }
        /**
         * Returns the entity id used to start a conversaion, if one is active.
         * 
         * Remember to check for validity if you're unsure there is an active conversation.
         * 
         * \return A valid AZ::EntityId if a conversation is active. Invalid otherwise.
         */
        virtual AZ::EntityId GetConversationOwner() const
        {
            return AZ::EntityId();
        }
    };

    class ConversationBusTraits : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using ConversationRequestBus = AZ::EBus<ConversationRequests, ConversationBusTraits>;
    using ConversationInterface = AZ::Interface<ConversationRequests>;

    class ConversationNotifications : public AZ::EBusTraits
    {
    public:
        AZ_RTTI(ConversationNotifications, "{E8160C73-598B-4F36-A39E-13F13A8033ED}");
        virtual ~ConversationNotifications() = default;

        // Notifies a conversation has started, allowing listeners to set up any conversation related systems, such
        // as the UI that displays the dialogue or system that plays associated audio.
        virtual void OnConversationStarted(const AZ::EntityId& /*entity*/)
        {
        }
        virtual void OnConversationEnded(const AZ::EntityId& /*entity*/)
        {
        }
        virtual void OnConversationAborted(const AZ::EntityId& /*entity*/)
        {
        }
        /**
         * Notifies of potential responses to the last OnDialogue notification.
         * 
         * \param availableResponses A container of choices that are available for selection. Can be empty.
         */
        virtual void OnChoiceAvailable([[maybe_unused]] const AZStd::vector<DialogueData>& availableResponses)
        {
        }
        virtual void OnChoiceSelected()
        {
        }
        // Notifies that a dialogue was sent (e.g. spoken). This is usually the time to display the associated
        // text on screen, play the associated audio, etc.
        virtual void OnDialogue(const DialogueData&)
        {
        }
    };

    using ConversationNotificationBus = AZ::EBus<ConversationNotifications>;

} // namespace Conversation
