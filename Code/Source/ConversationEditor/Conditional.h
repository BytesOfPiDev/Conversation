#pragma once

#include <AtomToolsFramework/DynamicProperty/DynamicProperty.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/map.h>
#include <ScriptEvents/ScriptEvent.h>

namespace ConversationEditor
{
    class ConditionalParameter
    {
        AZStd::string m_name;
        AZStd::any m_value;
    };

    class Conditional
    {
    public:
        AZ_TYPE_INFO(Conditional, "{ECD44736-F94B-4282-BFF3-0F6FE756097B}");
        AZ_CLASS_ALLOCATOR(Conditional, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        using ParameterMap = AZStd::map<AZ::Crc32, AtomToolsFramework::DynamicProperty>;

        AZ::Crc32 GetFunctionId() const
        {
            return m_functionId;
        }

        ParameterMap GetParameters() const
        {
            return m_parameters;
        }

    private:
        AZ::Crc32 m_functionId;
        ParameterMap m_parameters;
    };

    class ConditionalFunction
    {
    private:
        AZ::Crc32 m_functionId;
        AZStd::string m_functionName;

    };



    class ConditionManager
    {
    public:
        ConditionManager();

    protected:
        AZStd::vector<ConditionalFunction> BuildFunctions(ScriptEvents::ScriptEvent const * const scriptEvent);

    private:
        AZStd::vector<Conditional> m_conditionalFunctions;
    };

} // namespace ConversationEditor
