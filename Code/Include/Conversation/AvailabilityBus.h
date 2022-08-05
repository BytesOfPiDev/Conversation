#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Conversation
{
    class AvailabilityRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Crc32;

        virtual ~AvailabilityRequests() = default;

        virtual bool IsAvailable()
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
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorAvailabilityRequestBusHandler, "{D6037DA0-68CD-40F3-AB56-C91A6E7F4C3D}", AZ::SystemAllocator, IsAvailable);

        bool IsAvailable() override
        {
            bool result = false;
            CallResult(result, FN_IsAvailable);
            return result;
        }
    };

} // namespace Conversation