#pragma once

#include "AzCore/Component/TickBus.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "Conversation/DialogueData.h"

namespace Conversation
{

    AZ_ENUM(VMState, Inactive, Idle, Busy);

    AZ_ENUM(Instruction, OpReturn);

    struct VMData
    {
    };

    class ConversationVM : protected AZ::TickBus::Handler
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(ConversationVM);
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(ConversationVM);

        static void Reflect(AZ::ReflectContext* context);

        ConversationVM() = default;
        ~ConversationVM() override = default;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        VMState m_state{ Inactive };
    };

    struct StartConversationData
    {
    };

} // namespace Conversation
