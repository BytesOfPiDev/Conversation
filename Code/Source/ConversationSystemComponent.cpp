#include <ConversationSystemComponent.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/ConversationAsset.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponent.h>
#include <Conversation/DialogueScript.h>
#include <Conversation/AvailabilityBus.h>

namespace Conversation
{
    class BehaviorDialogueScriptRequestBusHandler
        : public DialogueScriptRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorDialogueScriptRequestBusHandler, "{168DA145-68E2-4D49-BCE7-3BAE5589C3D1}", AZ::SystemAllocator, RunDialogueScript);

        void RunDialogueScript() override
        {
            Call(FN_RunDialogueScript);
        }
    };

    void ConversationSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        DialogueData::Reflect(context);

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
            behaviorContext->EnumProperty<static_cast<int>(ConversationStates::Aborting)>("ConversationStatus_Aborting");
            behaviorContext->EnumProperty<static_cast<int>(ConversationStates::Active)>("ConversationStatus_Active");
            behaviorContext->EnumProperty<static_cast<int>(ConversationStates::Ending)>("ConversationStatus_Ending");
            behaviorContext->EnumProperty<static_cast<int>(ConversationStates::Inactive)>("ConversationStatus_Inactive");
            behaviorContext->EnumProperty<static_cast<int>(ConversationStates::Starting)>("ConversationStatus_Starting");

            const AZ::BehaviorParameterOverrides startConversationEntityIdParam = {
                "EntityId", "The Id of an entity that contains a conversation component that will be used to start the conversation."
            };

            behaviorContext->EBus<ConversationRequestBus>("ConversationRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System");

            behaviorContext->EBus<AvailabilityRequestBus>("AvailabilityRequestBus")
                ->Handler<BehaviorAvailabilityRequestBusHandler>();

            behaviorContext->EBus<DialogueScriptRequestBus>("DialogueScriptRequestBus")->Handler<BehaviorDialogueScriptRequestBusHandler>();
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

        auto test = aznew DialogueComponent();
        delete test;
        test = nullptr;
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
        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        m_conversationAssetHandler = AZStd::make_unique<ConversationAssetHandler>(
            "Conversation Asset", "Conversation System", ".conversation", AZ::AzTypeInfo<ConversationAsset>::Uuid(), serializeContext);
        AZ_Assert(m_conversationAssetHandler, "Unable to create conversation asset handler.");
    }

    void ConversationSystemComponent::Activate()
    {
        const AZ::Data::AssetType conversationAssetTypeId = azrtti_typeid<ConversationAsset>();
        if (!AZ::Data::AssetManager::Instance().GetHandler(conversationAssetTypeId))
        {
            AZ_Assert(
                m_conversationAssetHandler,
                "Conversation asset handler is null! It should have been created in the Init function already.");
            m_conversationAssetHandler->Register();
        }

        ConversationRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void ConversationSystemComponent::Deactivate()
    {
        if (!AZ::Data::AssetManager::Instance().IsReady() && m_conversationAssetHandler)
        {
            m_conversationAssetHandler->Unregister();
        }

        AZ::TickBus::Handler::BusDisconnect();
        ConversationRequestBus::Handler::BusDisconnect();
    }

    void ConversationSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }
} // namespace Conversation
