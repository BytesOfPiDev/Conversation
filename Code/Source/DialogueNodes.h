#pragma once

#include <Conversation/DialogueComponentBus.h>
#include <ScriptCanvas/Core/NodeFunctionGeneric.h>

namespace Conversation
{
    namespace Nodes
    {
        static constexpr const char* k_categoryName = "Conversation";

        AZStd::tuple<ScriptCanvas::Data::BooleanType, DialogueData> FindDialogueById(
            const DialogueId dialogueId, AZ::EntityId dialogueComponentEntityId);

        SCRIPT_CANVAS_GENERIC_FUNCTION_MULTI_RESULTS_NODE(
            FindDialogueById,
            k_categoryName,
            "{C0E6B13D-FA10-472B-A50F-B525F00B5D80}",
            "Tries to find the given DialogueId on the given EntityId's DialogueComponent.",
            "DialogueId",
            "DialogueComponentEntityId");

        using Registrar = ScriptCanvas::RegistrarGeneric<FindDialogueByIdNode>;

    } // namespace Nodes
} // namespace Conversation
