#include <DialogueNodes.h>

namespace Conversation
{
    namespace Nodes
    {
        AZStd::tuple<ScriptCanvas::Data::BooleanType, DialogueData> FindDialogueById(
            const DialogueId dialogueId, AZ::EntityId dialogueComponentEntityId)
        {
            DialogueData result = {};
            DialogueComponentRequestBus::EventResult(
                result, dialogueComponentEntityId, &DialogueComponentRequestBus::Events::FindDialogue, dialogueId);

            return AZStd::make_tuple(result.IsValid(), result);
        }
    } // namespace Nodes
} // namespace Conversation
