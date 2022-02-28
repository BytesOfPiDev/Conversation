
#include <ConversationSystemComponent.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/AvailabilityBus.h>
#include <Conversation/ConversationBus.h>
#include <DialogueLibrary.h>

namespace Conversation
{
    class BehaviorConversationNotificationBusHandler
        : public ConversationNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorConversationNotificationBusHandler,
            "{D5680C71-684A-451E-823F-DEE4F17D1DA9}",
            AZ::SystemAllocator,
            OnConversationStarted,
            OnConversationEnded,
            OnConversationAborted,
            OnChoiceAvailable,
            OnChoiceSelected,
            OnDialogue);

        void OnConversationStarted(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationStarted, entity);
        }

        void OnConversationEnded(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationEnded, entity);
        }

        void OnConversationAborted(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationAborted, entity);
        }

        void OnChoiceAvailable(const AZStd::vector<DialogueData>& choices) override
        {
            Call(FN_OnChoiceAvailable, choices);
        }

        void OnChoiceSelected() override
        {
            Call(FN_OnChoiceSelected);
        }

        void OnDialogue(const DialogueData& dialogueData) override
        {
            Call(FN_OnDialogue, dialogueData);
        }
    };

    void ConversationSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        DialogueLibrary::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<ConversationSystemComponent>("Conversation", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<AvailabilityNotificationBus>("AvailabilityNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Handler<BehaviorAvailabilityNotificationBusHandler>();
        }
    }

    void ConversationSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ConversationSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    ConversationSystemComponent::ConversationSystemComponent()
    {
        if (ConversationInterface::Get() == nullptr)
        {
            ConversationInterface::Register(this);
        }
    }

    ConversationSystemComponent::~ConversationSystemComponent()
    {
        if (ConversationInterface::Get() == this)
        {
            ConversationInterface::Unregister(this);
        }
    }

    void ConversationSystemComponent::Init()
    {
        AZ::EnvironmentVariable<ScriptCanvas::NodeRegistry> nodeRegistryVariable =
            AZ::Environment::FindVariable<ScriptCanvas::NodeRegistry>(ScriptCanvas::s_nodeRegistryName);
        if (nodeRegistryVariable)
        {
            ScriptCanvas::NodeRegistry& nodeRegistry = nodeRegistryVariable.Get();
            Conversation::DialogueLibrary::InitNodeRegistry(nodeRegistry);
        }
    }

    void ConversationSystemComponent::Activate()
    {
        ConversationRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void ConversationSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        ConversationRequestBus::Handler::BusDisconnect();
    }

    void ConversationSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Conversation
