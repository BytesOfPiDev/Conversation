#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Conversation
{
    class AvailabilityNotifications : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Uuid;

        virtual ~AvailabilityNotifications() = default;

        virtual bool OnAvailabilityCheck()
        {
            return false;
        }
    };
    
    using AvailabilityNotificationBus = AZ::EBus<AvailabilityNotifications>;

    class AvailabilityRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::Crc32;

        static void Reflect(AZ::ReflectContext* context);

        virtual ~AvailabilityRequests() = default;

        virtual bool CheckAvailability(const AZStd::vector<AZStd::any> args);
    };

    using AvailabilityRequestBus = AZ::EBus<AvailabilityRequests>;

    class BehaviorAvailabilityNotificationBusHandler
        : public AvailabilityNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorAvailabilityNotificationBusHandler, "{D6037DA0-68CD-40F3-AB56-C91A6E7F4C3D}", AZ::SystemAllocator, OnAvailabilityCheck);

        bool OnAvailabilityCheck() override
        {
            bool result = false;
            CallResult(result, FN_OnAvailabilityCheck);
            return result;
        }
    };


} // namespace Conversation
