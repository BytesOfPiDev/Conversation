#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/RTTI/BehaviorContext.h"

namespace Conversation
{
    class AvailabilityRequests : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(AvailabilityRequests); // NOLINT
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        AvailabilityRequests() = default;
        virtual ~AvailabilityRequests() = default;

        virtual auto IsAvailable(AZStd::string_view const) -> bool
        {
            return false;
        }
    };

    using AvailabilityRequestBus = AZ::EBus<AvailabilityRequests>;

    class BehaviorAvailabilityRequestBusHandler
        : public AvailabilityRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER( // NOLINT
            BehaviorAvailabilityRequestBusHandler,
            "{D6037DA0-68CD-40F3-AB56-C91A6E7F4C3D}",
            AZ::SystemAllocator,
            IsAvailable);

        auto IsAvailable(AZStd::string_view const availabilityId)
            -> bool override
        {
            bool result{};
            CallResult(result, FN_IsAvailable, availabilityId);
            return result;
        }
    };
} // namespace Conversation
