#include "DialogueLibrary.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include "AddDialogueNodeable.h"

namespace Conversation
{
    void DialogueLibrary::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueLibrary, ScriptCanvas::Library::LibraryDefinition>()->Version(1);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueLibrary>("Dialogue", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/ScriptCanvas/Libraries/Math.png");
            }

        }
    }
    void DialogueLibrary::InitNodeRegistry(ScriptCanvas::NodeRegistry& nodeRegistry)
    {
        ScriptCanvas::Library::AddNodeToRegistry<DialogueLibrary, Nodes::DialogueNodeableNode>(nodeRegistry);
    }
    AZStd::vector<AZ::ComponentDescriptor*> DialogueLibrary::GetComponentDescriptors()
    {
        return AZStd::vector<AZ::ComponentDescriptor*>({ Nodes::DialogueNodeableNode::CreateDescriptor() });
    }
} // namespace Conversation
