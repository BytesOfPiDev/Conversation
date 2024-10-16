#include "ConversationEditorSystemComponent.h"
#include "ActionManager/HotKey/HotKeyManagerInterface.h"
#include "ActionManager/Menu/MenuManagerInterface.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Utils/Utils.h"
#include "Editor/ActionManagerIdentifiers/EditorContextIdentifiers.h"

#include "AssetBuilderSDK/AssetBuilderSDK.h"
#include "Atom/RHI/Factory.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/Utils/Utils.h"
#include "AzToolsFramework/API/EditorAssetSystemAPI.h"
#include "AzToolsFramework/ActionManager/Action/ActionManagerInterface.h"
#include "AzToolsFramework/ActionManager/HotKey/HotKeyManager.h"
#include "AzToolsFramework/Editor/ActionManagerIdentifiers/EditorContextIdentifiers.h"
#include "Editor/ActionManagerIdentifiers/EditorMenuIdentifiers.h"

namespace ConversationEditor
{
    constexpr auto ConversationCanvasActionIdentifier =
        "bop.action.tools.conversation_canvas";

    void ConversationEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext
                ->Class<
                    ConversationEditorSystemComponent,
                    ConversationSystemComponent>()
                ->Version(0)
                ->Attribute(
                    AZ::Edit::Attributes::SystemComponentTags,
                    AZStd::vector<AZ::Crc32>({
                        AssetBuilderSDK::ComponentTags::AssetBuilder,
                    }));

            if (AZ::EditContext* editContext =
                    serializeContext->GetEditContext())
            {
                editContext
                    ->Class<ConversationEditorSystemComponent>(
                        "ConversationEditor",
                        "Handles editing conversation files.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::AppearsInAddComponentMenu,
                        AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    ConversationEditorSystemComponent::ConversationEditorSystemComponent()
        : m_conversationEditorNodeManager(AZ_CRC_CE("ConversationEditor"))
    {
    }

    ConversationEditorSystemComponent::~ConversationEditorSystemComponent() =
        default;

    void ConversationEditorSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetRequiredServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void ConversationEditorSystemComponent::GetDependentServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType&
            dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
        dependent.push_back(AZ_CRC_CE("ConversationAssetBuilderService"));
    }

    void ConversationEditorSystemComponent::Init()
    {
        ConversationSystemComponent::Init();
    }

    void ConversationEditorSystemComponent::Activate()
    {
        ConversationSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::
            BusConnect();
    }

    void ConversationEditorSystemComponent::Deactivate()
    {
        ConversationSystemComponent::Deactivate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::
            BusDisconnect();
    }

    void ConversationEditorSystemComponent::OnActionRegistrationHook()
    {
        auto actionManagerInterface =
            AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        AZ_Assert(
            actionManagerInterface,
            "EditorConversationSystemComponent - could not get "
            "ActionManagerInterface");

        auto hotKeyManagerInterface =
            AZ::Interface<AzToolsFramework::HotKeyManagerInterface>::Get();
        AZ_Assert(
            hotKeyManagerInterface,
            "EditorConversationSystemComponent - could not get "
            "HotKeyManagerInterface");

        {
            AzToolsFramework::ActionProperties actionProperties;
            actionProperties.m_name = "Conversation Canvas (prototype)";
            actionProperties.m_iconPath =
                ":/ConversationCanvas/toolbar_icon.svg";

            auto outcome = actionManagerInterface->RegisterAction(
                EditorIdentifiers::MainWindowActionContextIdentifier,
                ConversationCanvasActionIdentifier,
                actionProperties,
                [this]()
                {
                    OpenConversationCanvas("");
                });

            AZ_Assert(
                outcome.IsSuccess(),
                "Failed to RegisterAction %s",
                ConversationCanvasActionIdentifier);

            outcome = hotKeyManagerInterface->SetActionHotKey(
                ConversationCanvasActionIdentifier, "Ctrl+Shift+D");
            AZ_Assert(
                outcome.IsSuccess(),
                "Failed to ActionHotKey for %s",
                ConversationCanvasActionIdentifier);
        }
    }

    void ConversationEditorSystemComponent::OnMenuBindingHook()
    {
        auto actionManagerInterface =
            AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
        AZ_Assert(
            actionManagerInterface,
            "EditorConversationSystemComponent - could not get "
            "ActionManagerInterface");

        auto menuManagerInterface =
            AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
        AZ_Assert(
            menuManagerInterface,
            "EditorConversationSystemComponent - could not get "
            "MenuManagerInterface");

        {
            auto outcome = menuManagerInterface->AddActionToMenu(
                EditorIdentifiers::ToolsMenuIdentifier,
                ConversationCanvasActionIdentifier,
                actionManagerInterface->GenerateActionAlphabeticalSortKey(
                    ConversationCanvasActionIdentifier));

            AZ_Assert(
                outcome.IsSuccess(),
                "Failed to AddAction %s to Menu %s",
                ConversationCanvasActionIdentifier,
                EditorIdentifiers::ToolsMenuIdentifier.data());
        }
    }

    void ConversationEditorSystemComponent::OpenConversationCanvas(
        AZStd::string const& sourcePath)
    {
        QStringList arguments;
        arguments.append(sourcePath.c_str());

        // Use the same RHI as the main Canvas
        AZ::Name apiName = AZ::RHI::Factory::Get().GetName();
        if (!apiName.IsEmpty())
        {
            arguments.append(QString("--rhi=%1").arg(apiName.GetCStr()));
        }

        AZ::IO::FixedMaxPathString projectPath(AZ::Utils::GetProjectPath());
        if (!projectPath.empty())
        {
            arguments.append(
                QString("--project-path=%1").arg(projectPath.c_str()));
        }

        AZ_Info( 
            "ConversationCanvas",
            "Launching Conversation Canvas");
        AtomToolsFramework::LaunchTool("ConversationCanvas", arguments);
    }

} // namespace ConversationEditor
