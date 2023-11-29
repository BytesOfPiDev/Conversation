#pragma once

#include "AzCore/RTTI/ReflectContext.h"

namespace Conversation
{
    class ConditionId
    {
    public:
        AZ_TYPE_INFO_WITH_NAME_DECL(ConditionId); // NOLINT
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_DEFAULT_COPY_MOVE(ConditionId); // NOLINT

        static void Reflect(AZ::ReflectContext* context);
        ConditionId() = default;
        ~ConditionId() = default;
    };

    struct ConditionFunction
    {
        AZ_TYPE_INFO(ConditionFunction, "3C516F9D-7F5A-4326-800E-B5C8AF5B9F3E"); // NOLINT
        static void Reflect(AZ::ReflectContext* context);

        AZ::Name m_functionName{};
        AZStd::string m_instruction{};
        AZStd::string m_functionStringName{};
    };
} // namespace Conversation
