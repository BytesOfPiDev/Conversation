#include "Tools/EditorInstructions.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace ConversationEditor
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(EditorInstructions, "Instructions", "79CBBD47-DCFA-4DD3-824F-D08E94A9C673"); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(EditorInstructions, AZ::SystemAllocator, 0); // NOLINT

    void EditorInstructions::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<EditorInstructions>()->Version(0)->Field("Value", &EditorInstructions::m_instructions);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<EditorInstructions>("Instructions", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorInstructions::m_instructions, "Value", "");
            }
        }
    }
} // namespace ConversationEditor
