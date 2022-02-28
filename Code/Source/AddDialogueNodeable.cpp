#include <AddDialogueNodeable.h>

#include <Conversation/DialogueComponentBus.h>

namespace Conversation
{
    AZ::Uuid DialogueNodeable::AddDialogue([[maybe_unused]] AZ::Uuid parentDialogueId, [[maybe_unused]] AZStd::string speaker, [[maybe_unused]] AZStd::string text)
    {
        DialogueData newDialogueData = DialogueData(true);
        newDialogueData.SetSpeaker(speaker);
        newDialogueData.SetScript(text);

        DialogueId result = DialogueId::CreateNull();
        DialogueComponentRequestBus::EventResult(result, GetEntityId(), &DialogueComponentRequestBus::Events::AddDialogue, newDialogueData, parentDialogueId);

        // Connect to availability bus
        return result;
    }
}
