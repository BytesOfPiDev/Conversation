#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Conversation/DialogueData.h>

namespace Conversation
{
    class DialogueComponentRequests : public AZ::ComponentBus
    {
    public:
        ~DialogueComponentRequests() override = default;

    public: // Requests
        virtual DialogueId AddDialogue(const DialogueData /*dialogueData*/, const DialogueId& /*parentDialogue*/)
        {
            return DialogueId::CreateNull();
        }

        virtual void ClearData()
        {
        }

        virtual ConversationData GetConversationData() const
        {
            return {};
        }
    };

    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

} // namespace Conversation
