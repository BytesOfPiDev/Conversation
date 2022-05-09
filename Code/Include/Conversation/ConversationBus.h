
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


} // namespace Conversation
