#pragma once

#include "AzCore/Component/EntityId.h"
#include "AzCore/EBus/EBus.h"

namespace Conversation
{
    class CompanionScriptRequests : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(CompanionScriptRequests);

        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;

        using BusIdType = AZ::EntityId;

        CompanionScriptRequests() = default;
        virtual ~CompanionScriptRequests() = default;

        virtual void RunCompanionScript(AZStd::string_view nodeId) = 0;
    };

    using CompanionScriptRequestBus = AZ::EBus<CompanionScriptRequests>;

} // namespace Conversation
