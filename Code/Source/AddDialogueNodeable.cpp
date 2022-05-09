#include <AddDialogueNodeable.h>

#include <Conversation/DialogueComponentBus.h>

namespace Conversation
{
    AZ::Uuid DialogueNodeable::AddDialogue(
        const AZ::Uuid parentDialogueId,
        const ScriptCanvas::Data::StringType speaker,
        const ScriptCanvas::Data::StringType text,
        const ScriptCanvas::Data::EntityIDType dialogueComponentEntityId)
    {
        // Generates a random id when true is passed to the constructor.
        DialogueData newDialogueData = DialogueData(true);
        AZ_Assert(!newDialogueData.GetId().IsNull(), "The dialogue data must have a valid ID.");

        newDialogueData.SetSpeaker(speaker);
        newDialogueData.SetActorText(text);
        newDialogueData.SetAudioTrigger(m_audioTrigger);

        DialogueComponentRequestBus::Event(
            dialogueComponentEntityId, &DialogueComponentRequestBus::Events::AddDialogue, newDialogueData, parentDialogueId);

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
