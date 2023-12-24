#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/any.h>
#include <AzCore/std/containers/vector.h>
#include <AzQtComponents/Components/Widgets/Card.h>
#endif

namespace DialogueEditor
{
    struct Command
    {
        AZ::Crc32 CommandId;
        AZStd::vector<AZ::TypeId> ArgumentTypes;
        AZStd::vector<AZStd::any> ArgumentValues;
        AZ::TypeId ReturnType;
    };

    enum class CommandValueType
    {
        Number,
        String,
        Boolean,
        Tag
    };

    struct CommandParameter
    {
        AZ::TypeId ValueType;

        union {
            bool BooleanValue;
            float NumberValue;
            char* StringValue;
        };
    };

    struct CommandDefinition
    {
    public:
        // AZ_TYPE_INFO(CommandDefinition,
        // "{2C0630DF-0D51-4E16-B89E-B4AC947AEE79}"); static void
        // Reflect(AZ::ReflectContext* context);

        // private:
        AZ::Crc32 m_id;
        AZStd::string m_name;
        AZStd::string m_tooltip;
        AZStd::vector<CommandValueType> m_parameterTypes;

        AZStd::vector<AZ::TypeId> ArgumentTypes;
        AZ::TypeId ReturnType;
    };

    class AvailabilityCheckCard : public AzQtComponents::Card
    {
        Q_OBJECT;

    public:
        AvailabilityCheckCard(QWidget* parent = nullptr)
            : AzQtComponents::Card(parent)
        {
        }
        AvailabilityCheckCard(
            Command const& leftCommand,
            Command const& rightCommand,
            QWidget* parent = nullptr)
            : AzQtComponents::Card(parent)
            , m_left(leftCommand)
            , m_right(rightCommand)
        {
        }

    private:
        Command m_left;
        Command m_right;
    };

} // namespace DialogueEditor