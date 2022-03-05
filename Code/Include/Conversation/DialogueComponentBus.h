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
        virtual void AddDialogue(const DialogueData /*dialogueDataToAdd*/, const DialogueId& /*parentDialogueId*/)
        {

        }

        virtual void ClearData()
        {
        }

        virtual AZ::Data::Asset<ConversationAsset> GetConversationData() const
        {
            return {};
        }
    };

    using DialogueComponentRequestBus = AZ::EBus<DialogueComponentRequests>;

} // namespace Conversation
