#include "Conditions/ComparisonOp.h"

#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

namespace Conversation
{

    void ComparisonOp::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ComparisonOp>()->Version(0)->Field("Op", &ComparisonOp::m_opNames);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<ComparisonOp>("ComparisonOp", "");
            }
        }
    }
} // namespace Conversation
