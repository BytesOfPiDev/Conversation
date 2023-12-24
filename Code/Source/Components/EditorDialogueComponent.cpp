#include "Components/EditorDialogueComponent.h"

#include "AzCore/Console/ILogger.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/std/ranges/ranges_algorithm.h"
#include "AzFramework/Script/ScriptComponent.h"
#include "AzToolsFramework/Entity/EditorEntityContextBus.h"
#include "ToolsComponents/EditorComponentBase.h"
#include "cstdlib"

#include "Conversation/ConversationAsset.h"
#include "Conversation/DialogueComponent.h"

namespace ConversationEditor
{

    void EditorDialogueComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize
                ->Class<
                    EditorDialogueComponent,
                    AzToolsFramework::Components::EditorComponentBase>()
                ->Version(1)
                ->Field("Config", &EditorDialogueComponent::m_config);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<EditorDialogueComponent>("Dialogue", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::Category, "BoP/Conversation")
                    ->Attribute(
                        AZ::Edit::Attributes::AppearsInAddComponentMenu,
                        AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &EditorDialogueComponent::m_config,
                        "Configuration",
                        "");
            }
        }
    }

    void EditorDialogueComponent::Init()
    {
    }

    void EditorDialogueComponent::Activate()
    {
    }

    void EditorDialogueComponent::Deactivate()
    {
    }

    void EditorDialogueComponent::DisplayEntityViewport(
        AzFramework::ViewportInfo const& viewportInfo,
        AzFramework::DebugDisplayRequests& debugDisplay)
    {
    }

    void EditorDialogueComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("EditorDialogueService"));
        provided.push_back(AZ_CRC_CE("DialogueService"));
    }
    void EditorDialogueComponent::GetDependentServices(
        AZ::ComponentDescriptor::DependencyArrayType& /*dependent*/)
    {
    }
    void EditorDialogueComponent::GetRequiredServices(
        AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TagService"));
    }
    void EditorDialogueComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("EditorDialogueService"));
        incompatible.push_back(AZ_CRC_CE("DialogueService"));
    }

    void EditorDialogueComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        AZ_Assert( // NOLINT
            gameEntity,
            "The game entity should not be null!");

        auto* dialogueComponent =
            gameEntity->CreateComponent<Conversation::DialogueComponent>();
        if (!dialogueComponent)
        {
            AZLOG_FATAL( // NOLINT
                "An EditorDialogueComponent is unable to build a game entity "
                "because it could not create a Dialogue Component instance.");
            return;
        }

        dialogueComponent->SetConfiguration(m_config);
    }

} // namespace ConversationEditor
