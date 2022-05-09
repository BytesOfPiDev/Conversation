#include <Conversation/AvailabilityBus.h>

#include <AzCore/RTTI/BehaviorContext.h>

namespace Conversation
{
    class BehaviorAvailabilityRequestBusHandler
        : public AvailabilityRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorAvailabilityRequestBusHandler, "{0A6CDB0E-1276-41CD-B085-0808F3A4F0CF}", AZ::SystemAllocator, CheckAvailability);

        bool CheckAvailability(const AZStd::vector<AZStd::any> args) override
        {
            bool result = false;
            CallResult(result, FN_CheckAvailability, args);

            return result;
        }
    };

    void AvailabilityRequests::Reflect(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<AvailabilityRequestBus>("AvailabilityRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Conversation System")
                ->Handler<BehaviorAvailabilityRequestBusHandler>();
        }
    }

    bool AvailabilityRequests::CheckAvailability(const AZStd::vector<AZStd::any> args)
    {
        return false;
    }
} // namespace Conversation
