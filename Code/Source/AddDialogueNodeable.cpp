#include <AddDialogueNodeable.h>

#include <Conversation/DialogueComponentBus.h>

namespace Conversation
{
    AZ::Uuid DialogueNodeable::AddDialogue(
        [[maybe_unused]] AZ::Uuid parentDialogueId, [[maybe_unused]] AZStd::string speaker, [[maybe_unused]] AZStd::string text)
    {
        // Generates a random id when true is passed to the constructor.
        DialogueData newDialogueData = DialogueData(true);
        AZ_Assert(!newDialogueData.GetId().IsNull(), "The dialogue data must have a valid ID.");

        newDialogueData.SetSpeaker(speaker);
        newDialogueData.SetActorText(text);

        DialogueComponentRequestBus::Event(
            GetEntityId(), &DialogueComponentRequestBus::Events::AddDialogue, newDialogueData, parentDialogueId);

        DialogueScriptRequestBus::Handler::BusConnect(newDialogueData.GetId());
        return newDialogueData.GetId();
    }

    void DialogueNodeable::RunDialogueScript()
    {
        CallOnDialogue();
    }

    void DialogueNodeable::OnDeactivate()
    {
        DialogueScriptRequestBus::Handler::BusDisconnect();
        ScriptCanvas::Nodeable::OnDeactivate();
    }

} // namespace Conversation
