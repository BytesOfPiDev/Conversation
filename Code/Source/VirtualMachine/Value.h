#pragma once

#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"

namespace Conversation
{
    template<typename ValueType>
    class ScriptValue
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(ScriptValue);
        AZ_CLASS_ALLOCATOR_DECL;

        ScriptValue(ValueType initialValue = {})
            : m_value{ initialValue } {};

        static void Reflect(AZ::ReflectContext* context)
        {
            if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serialize->Class<ScriptValue>()->Version(0);

                if (AZ::EditContext* editContext = serialize->GetEditContext())
                {
                    editContext->Class<ScriptValue>("Value", "")->ClassElement(AZ::Edit::ClassElements::EditorData, "");
                }
            }
        }

    private:
        ValueType m_value;
    };
} // namespace Conversation
