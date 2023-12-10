#pragma once

#include "AzCore/Component/Component.h"

#include "AzCore/Component/ComponentBus.h"
#include "AzCore/Outcome/Outcome.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    class ConversationAssetRefComponentRequests
        : public AZ::ComponentBus
        , public IConversationAsset
    {
    public:
        AZ_RTTI(ConversationAssetRefComponentRequests, "{EF3E6310-6459-4964-9742-C4EEFF352B68}", IConversationAsset); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationAssetRefComponentRequests); // NOLINT

        ConversationAssetRefComponentRequests() = default;
        ~ConversationAssetRefComponentRequests() override = default;
    };

    using ConversationAssetRefComponentRequestBus = AZ::EBus<ConversationAssetRefComponentRequests, AZ::ComponentBus>;
} // namespace Conversation
