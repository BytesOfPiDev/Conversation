#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/Interface/Interface.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{

    // NOTE: Not used.
    struct ConversationAssetRequestBusTraits : public AZ::EBusTraits
    {
        using MutexType = AZ::NullMutex;
        using EventQueueMutexType = AZ::NullMutex;

        static constexpr bool EnableEventQueue = true;
        static constexpr bool EventQueueingActiveByDefault = true;
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;
    };

    using ConversationAssetBus =
        AZ::EBus<IConversationAsset, ConversationAssetRequestBusTraits>;
    using ConversationAssetInterface = AZ::Interface<IConversationAsset>;
} // namespace Conversation
