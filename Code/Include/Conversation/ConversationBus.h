
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

        virtual void SelectResponseByNumber(const AZ::u8)
        {
        }
        virtual void SelectResponseById(const DialogueId&)
        {
        }

        virtual AZ::EntityId GetConversationOwner() const
        {
            return AZ::EntityId();
        }

        virtual bool IsSpeakerThePlayer(const DialogueData& dialogueData)
        {
            return dialogueData.GetSpeaker() == SPEAKERTAG_PLAYER;
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

        virtual void OnConversationStarted(const AZ::EntityId& /*entity*/)
        {
        }
        virtual void OnConversationEnded(const AZ::EntityId& /*entity*/)
        {
        }
        virtual void OnConversationAborted(const AZ::EntityId& /*entity*/)
        {
        }
        virtual void OnChoiceAvailable(const AZStd::vector<DialogueData>&)
        {
        }
        virtual void OnChoiceSelected()
        {
        }
        virtual void OnDialogue(const DialogueData&)
        {
        }
    };

    using ConversationNotificationBus = AZ::EBus<ConversationNotifications>;

} // namespace Conversation
