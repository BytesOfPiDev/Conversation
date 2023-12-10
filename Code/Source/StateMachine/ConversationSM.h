#pragma once

#include "AzCore/Component/EntityId.h"
#include "AzCore/Component/TickBus.h"

namespace Conversation
{
    class ConversationSM : protected AZ::TickBus::Handler
    {
    public:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        auto TryToStartConversation(AZ::EntityId const& initiatingEntityId) -> bool;
    };
} // namespace Conversation
