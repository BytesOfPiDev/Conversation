
#include <ConversationSystemComponent.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/AvailabilityBus.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponent.h>
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
        DialogueData::Reflect(context);
        DialogueLibrary::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext
                    ->Class<ConversationSystemComponent>("Conversation", "[Description of functionality provided by this System Component]")
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

            const AZ::BehaviorParameterOverrides startConversationEntityIdParam = {
                "EntityId", "The Id of an entity that contains a conversation component that will be used to start the conversation."
            };

            // startConversationParams.

            behaviorContext->EBus<ConversationRequestBus>("ConversationRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Event("StartConversation", &ConversationRequestBus::Events::StartConversation, { startConversationEntityIdParam })
                ->Event("EndConversation", &ConversationRequestBus::Events::EndConversation)
                ->Event("AbortConversation", &ConversationRequestBus::Events::AbortConversation)
                ->Event("SelectResponseById", &ConversationRequestBus::Events::SelectResponseById)
                ->Event("SelectResponseByNumber", &ConversationRequestBus::Events::SelectResponseByNumber);

            behaviorContext->EBus<ConversationNotificationBus>("ConversationNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Handler<BehaviorConversationNotificationBusHandler>();
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

    void ConversationSystemComponent::AbortConversation()
    {
    }

    void ConversationSystemComponent::EndConversation()
    {
    }

    void ConversationSystemComponent::StartConversation(const AZ::EntityId entityId)
    {
        // Refuse to start a conversation if the system is not currently inactive.
        if (m_currentConversationStatus != ConversationStatus::Inactive)
        {
            return;
        }

        // We set the status to starting so external code can know we're in the middle
        // of setting up the conversation. It lets them know not to trust any information
        // while in this state.
        m_currentConversationStatus = ConversationStatus::Starting;

        // ConversationData resultConversationData = {};
        m_activeConversationData = AZStd::make_shared<ConversationData>();
        DialogueComponentRequestBus::EventResult(
            *m_activeConversationData, entityId, &DialogueComponentRequestBus::Events::GetConversationData);

        AZStd::unordered_set<DialogueId> availableIds;

        // Check each starting id and store the available ones.
        for (DialogueId startingId : m_activeConversationData->GetStartingIds())
        {
            // Dialogues are available by default. The only time one will not be available is if an
            // availability check was setup somewhere, which does some logic that returns true or false.
            // Remember, the following variable is only modified if the EBus call is successful.
            bool resultDialogueIsAvailable = true;
            AvailabilityNotificationBus::EventResult(
                resultDialogueIsAvailable, startingId, &AvailabilityNotificationBus::Events::OnAvailabilityCheck);

            availableIds.insert(startingId);
        }

        AZ_Warning("ConversationSystemComponent", !availableIds.empty(), "No starting IDs were available. Cannot start conversation.");

        if (availableIds.empty())
        {
            m_currentConversationStatus = ConversationStatus::Inactive;
            return;
        }

        // For now, we're going to choose the first one.
        // Later, we may use some logic to decide which to use when multiple are available.
        const DialogueId chosenDialogueId = *availableIds.begin();
        // Try to retrieve the dialogue using the id. Should never fail, but potentially could.
        if (auto outcome = m_activeConversationData->GetDialogueById(chosenDialogueId))
        {
            m_activeDialogue = outcome.IsSuccess() ? AZStd::make_shared<DialogueData>(outcome.GetValue()) : nullptr;
            m_currentConversationStatus = ConversationStatus::Active;
            return;
        }

        // If we've made it this far, then we failed to get everything we needed to
        // start a conversation. Reset everything.
        m_activeConversationData = nullptr;
        m_activeDialogue = nullptr;
        m_currentConversationStatus = ConversationStatus::Inactive;
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
