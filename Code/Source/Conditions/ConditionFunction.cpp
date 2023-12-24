#include "Conditions/ConditionFunction.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

namespace Conversation
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(
        ConditionId,
        "ConditionId",
        "7FDBADB9-D48F-46BD-932E-5F1B09994B0F"); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(ConditionId, AZ::SystemAllocator, 0); // NOLINT

    void ConditionId::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConditionId>()->Version(0);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<ConditionId>("ConditionId", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }
    }

    void ConditionFunction::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConditionFunction>()
                ->Version(0)
                ->Field("FunctionName", &ConditionFunction::m_functionName)
                ->Field(
                    "FunctionNameString",
                    &ConditionFunction::m_functionStringName)
                ->Field(
                    "FunctionInstruction", &ConditionFunction::m_instruction);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<ConditionFunction>("ConditionFunction", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::Category, "BoP/Conversation")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &ConditionFunction::m_functionStringName,
                        "Function Name",
                        "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &ConditionFunction::m_instruction,
                        "Instruction",
                        "");
            }
        }
    }
} // namespace Conversation
