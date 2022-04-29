#pragma once

#include <Conversation/DialogueData.h>
#include <Conversation/DialogueScript.h>
#include <ScriptCanvas/CodeGen/NodeableCodegen.h>
#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Core/NodeableNode.h>

#include <Source/AddDialogueNodeable.generated.h>

namespace Conversation
{
    class DialogueNodeable
        : public ScriptCanvas::Nodeable
        , public DialogueScriptRequestBus::Handler
    {
        SCRIPTCANVAS_NODE(DialogueNodeable);

    public:
        DialogueNodeable() = default;
        DialogueNodeable(const DialogueNodeable&) = default;
        ~DialogueNodeable() override = default;

    protected:
        void RunDialogueScript() override;
        void OnDeactivate() override;
    };
} // namespace Conversation
