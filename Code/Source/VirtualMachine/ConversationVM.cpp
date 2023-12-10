#include "ConversationVM.h"

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(ConversationVM, "ConversationVM", ConversationVMTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(ConversationVM, AZ::SystemAllocator);

    void ConversationVM::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationVM>()->Version(0);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<ConversationVM>("ConversationVM", "")->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }
    }

    void ConversationVM::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (Inactive)
        {
            return;
        }


    }

} // namespace Conversation
