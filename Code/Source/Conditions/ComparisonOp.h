#pragma once

#include "AzCore/Name/Name.h"
#include "AzCore/RTTI/TypeInfo.h"
#include "AzCore/std/containers/vector.h"
#include "AzCore/std/string/string.h"

namespace Conversation
{
    struct ComparisonOp
    {
        AZ_TYPE_INFO(ComparisonOp, "123B4706-751C-432E-A1DA-7BEF4EA236D6"); // NOLINT
        static void Reflect(AZ::ReflectContext* context);

        AZStd::vector<AZ::Name> m_opNames;
    };
} // namespace Conversation
