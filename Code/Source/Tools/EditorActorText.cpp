#include "Tools/EditorActorText.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace ConversationEditor
{
    void EditorActorText::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<EditorActorText>()->Version(0)->Field("Text", &EditorActorText::m_text);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<EditorActorText>("Actor Text", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &EditorActorText::m_text, "Text", "");
            }
        }
    }
} // namespace ConversationEditor
