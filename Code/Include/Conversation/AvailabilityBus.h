#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Conversation
{
    constexpr auto MaxParameters = 5;

    class ConditionalRequests : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(ConditionalRequests); // NOLINT

        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        ConditionalRequests() = default;
        virtual ~ConditionalRequests() = default;

        virtual auto ExecuteConditional(AZStd::array<AZStd::any, MaxParameters>, AZStd::any) -> bool = 0;
    };

    using ConditionalRequestBus = AZ::EBus<ConditionalRequests>;

    class AvailabilityRequests : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(AvailabilityRequests); // NOLINT
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
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

        auto IsAvailable(AZStd::string_view const availabilityId) -> bool override
        {
            bool result{};
            CallResult(result, FN_IsAvailable, availabilityId);
            return result;
        }
    };

    class BehaviorConditionalRequestBusHandler
        : public ConditionalRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER( // NOLINT
            BehaviorConditionalRequestBusHandler,
            "88DD9D5E-9870-4761-A459-90320C04BEE6",
            AZ::SystemAllocator,
            ExecuteConditional);

        auto ExecuteConditional(AZStd::array<AZStd::any, MaxParameters> conditionParameters, AZStd::any conditionResult) -> bool override
        {
            bool executionResult{};
            CallResult(executionResult, FN_ExecuteConditional, conditionParameters, conditionResult);
            return executionResult;
        }
    };
} // namespace Conversation
