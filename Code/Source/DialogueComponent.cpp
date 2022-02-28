#include <Conversation/DialogueComponent.h>

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
//#include <Conversation/ConversationBus.h>

namespace Conversation
{
    DialogueComponent::~DialogueComponent()
    {

    }

    void DialogueComponent::Reflect(AZ::ReflectContext* context)
    {
        DialogueData::Reflect(context);

        constexpr static const char* DIALOGUE_SYSTEM_CATEGORY = "Dialogue System";
        constexpr static const char* DIALOGUE_COMPONENT_CATEGORY = "Dialogue System/DialogueComponent";

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<DialogueComponent, AZ::Component>()
                ->Version(0)
                ->Field("Owner", &DialogueComponent::m_owner);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueComponent>("Dialogue", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueComponent::m_owner, "Owner", "The actor that this dialogue applies to.");
            }
        }

        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            //behaviorContext->Class<DialogueComponent>()->RequestBus("DialogueComponentRequestBus");

            behaviorContext->EBus<DialogueComponentRequestBus>("DialogueComponentRequests")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Event("Add Dialogue", &DialogueComponentRequestBus::Events::AddDialogue, { { { "Dialogue Data", "" } } });
        }
    }

    void DialogueComponent::Init()
    {

    }

    void DialogueComponent::Activate()
    {
        ConversationNotificationBus::Handler::BusConnect();
        DialogueComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void DialogueComponent::Deactivate()
    {
        DialogueComponentRequestBus::Handler::BusDisconnect(GetEntityId());
        ConversationNotificationBus::Handler::BusDisconnect();
    }

    void DialogueComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("DialogueComponentService", 0x7f0fbed2));
    }

    void DialogueComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("DialogueComponentService", 0x7f0fbed2));
    }

    void DialogueComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_UNUSED(required);
    }

    void DialogueComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    DialogueId DialogueComponent::AddDialogue(
        const DialogueData& dialogueData, const DialogueId&)
    {
        AZ_UNUSED(dialogueData);

        return DialogueId::CreateNull();
    }

} // namespace Conversation
