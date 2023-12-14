#pragma once

#include "AzCore/Component/ComponentBus.h"
#include "Conversation/ConversationAsset.h"

namespace Conversation
{
    class ConversationAssetRefComponentRequests
        : public AZ::ComponentBus
        , public IConversationAsset
    {
    public:
        AZ_RTTI(ConversationAssetRefComponentRequests, "{EF3E6310-6459-4964-9742-C4EEFF352B68}", IConversationAsset); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationAssetRefComponentRequests); // NOLINT

        virtual auto SetConversationAsset(AZ::Data::Asset<ConversationAsset> replacementAsset) -> bool = 0;

        ConversationAssetRefComponentRequests() = default;
        ~ConversationAssetRefComponentRequests() override = default;
    };

    using ConversationAssetRefComponentRequestBus = AZ::EBus<ConversationAssetRefComponentRequests, AZ::ComponentBus>;
} // namespace Conversation
