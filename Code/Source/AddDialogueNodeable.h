#pragma once

#include <ScriptCanvas/CodeGen/NodeableCodegen.h>
#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Core/NodeableNode.h>

#include <Source/AddDialogueNodeable.generated.h>

namespace Conversation
{
    class DialogueNodeable : public ScriptCanvas::Nodeable
    {
        SCRIPTCANVAS_NODE(DialogueNodeable);

    public:
        DialogueNodeable() = default;
        DialogueNodeable(const DialogueNodeable&) = default;
        ~DialogueNodeable() override = default;
    };
} // namespace Conversation
