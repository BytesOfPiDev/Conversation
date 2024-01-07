#include "ConversationSystemComponent.h"

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/AvailabilityBus.h"
#include "Conversation/Constants.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationBus.h"
#include "Conversation/DialogueChunk.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData.h"
#include "Conversation/DialogueScript.h"
#include "Conversation/UniqueId.h"

namespace Conversation
{
    class BehaviorDialogueScriptRequestBusHandler
        : public CompanionScriptRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER( // NOLINT
            BehaviorDialogueScriptRequestBusHandler,
            "{168DA145-68E2-4D49-BCE7-3BAE5589C3D1}",
            AZ::SystemAllocator,
            RunCompanionScript);

        void RunCompanionScript(AZStd::string_view nodeId) override
        {
            Call(FN_RunCompanionScript, nodeId);
        }
    };

    void ReflectUniqueId(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<UniqueId>()->Version(2)->Field(
                "Value", &UniqueId::m_value);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<UniqueId>("UniqueId")
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(
                    AZ::Script::Attributes::Operator,
                    AZ::Script::Attributes::OperatorType::Equal)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventReturnType, true)
                ->Property("Value", BehaviorValueProperty(&UniqueId::m_value));
        }
    }

    void ReflectDialogueChunk(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DialogueChunk>()->Version(2)->Field(
                "Data", &DialogueChunk::m_data);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<DialogueChunk>("Dialogue Chunk", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::MultiLineEdit,
                        &DialogueChunk::m_data,
                        "Chunk",
                        "");
            }
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<DialogueChunk>()
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Constructor()
                ->Property(
                    "Data", &DialogueChunk::GetData, &DialogueChunk::SetData);
        }
    }

    void ConversationSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        ReflectUniqueId(context);
        ReflectDialogueChunk(context);
        DialogueData::Reflect(context);

        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationSystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext
                    ->Class<ConversationSystemComponent>(
                        "Conversation",
                        "[Description of functionality provided by this System "
                        "Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::AppearsInAddComponentMenu,
                        AZ_CRC_CE("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext
                ->EnumProperty<static_cast<int>(DialogueState::Aborting)>(
                    "ConversationStatus_Aborting");
            behaviorContext
                ->EnumProperty<static_cast<int>(DialogueState::Active)>(
                    "ConversationStatus_Active");
            behaviorContext
                ->EnumProperty<static_cast<int>(DialogueState::Ending)>(
                    "ConversationStatus_Ending");
            behaviorContext
                ->EnumProperty<static_cast<int>(DialogueState::Inactive)>(
                    "ConversationStatus_Inactive");
            behaviorContext
                ->EnumProperty<static_cast<int>(DialogueState::Starting)>(
                    "ConversationStatus_Starting");

            behaviorContext
                ->EBus<ConversationRequestBus>("ConversationRequestBus")
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true);

            behaviorContext
                ->EBus<AvailabilityRequestBus>("AvailabilityRequestBus")
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Handler<BehaviorAvailabilityRequestBusHandler>();

            behaviorContext
                ->EBus<CompanionScriptRequestBus>("DialogueScriptRequestBus")
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Handler<BehaviorDialogueScriptRequestBusHandler>();
        }
    }

    void ConversationSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ConversationSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType&
            dependent)
    {
        dependent.push_back(AZ_CRC_CE("ScriptCanvasService"));
        dependent.push_back(AZ_CRC_CE("AssetDatabaseService"));
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
        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(
            serializeContext,
            &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        m_conversationAssetHandler =
            AZStd::make_unique<ConversationAssetHandler>(
                "Conversation Asset",
                "Conversation System",
                ".conversation",
                AZ::AzTypeInfo<ConversationAsset>::Uuid(),
                serializeContext);
        AZ_Assert(
            m_conversationAssetHandler,
            "Unable to create conversation asset handler."); // NOLINT
    }

    void ConversationSystemComponent::Activate()
    {
        const AZ::Data::AssetType conversationAssetTypeId =
            azrtti_typeid<ConversationAsset>();
        if (!AZ::Data::AssetManager::Instance().GetHandler(
                conversationAssetTypeId))
        {
            AZ_Assert( // NOLINT(*-pro-type-vararg,
                       // *-bounds-array-to-pointer-decay)
                m_conversationAssetHandler,
                "Conversation asset handler is null! It should have been "
                "created in the Init function already.");
            m_conversationAssetHandler->Register();
        }

        ConversationRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void ConversationSystemComponent::Deactivate()
    {
        if (!AZ::Data::AssetManager::Instance().IsReady() &&
            m_conversationAssetHandler)
        {
            m_conversationAssetHandler->Unregister();
        }

        AZ::TickBus::Handler::BusDisconnect();
        ConversationRequestBus::Handler::BusDisconnect();
    }

    void ConversationSystemComponent::OnTick(
        [[maybe_unused]] float deltaTime,
        [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }
} // namespace Conversation
