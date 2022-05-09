#include "DialogueLibrary.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/DialogueNodes.h>

#include <AddDialogueNodeable.h>
#include <DialogueNodes.h>

namespace Conversation
{
    void DialogueLibrary::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueLibrary, ScriptCanvas::Library::LibraryDefinition>()->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueLibrary>("ConversationLibrary", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/ScriptCanvas/Libraries/Math.png");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            SCRIPT_CANVAS_GENERICS_TO_VM(Nodes::Registrar, DialogueLibrary, behaviorContext, Nodes::k_categoryName);
        }
    }
    void DialogueLibrary::InitNodeRegistry(ScriptCanvas::NodeRegistry& nodeRegistry)
    {
        ScriptCanvas::Library::AddNodeToRegistry<DialogueLibrary, Nodes::DialogueNodeableNode>(nodeRegistry);

        Nodes::Registrar::AddToRegistry<DialogueLibrary>(nodeRegistry);
    }
    AZStd::vector<AZ::ComponentDescriptor*> DialogueLibrary::GetComponentDescriptors()
    {
        auto descriptors = AZStd::vector<AZ::ComponentDescriptor*>();
        descriptors.push_back(Nodes::DialogueNodeableNode::CreateDescriptor());

        Nodes::Registrar::AddDescriptors(descriptors);
        return descriptors;
    }
} // namespace Conversation
