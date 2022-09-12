
#include <AzCore/Serialization/SerializeContext.h>

#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>

#include <ConversationEditorSystemComponent.h>
#include <ConversationWidget.h>

#include <ConversationEditor/ConversationDocument.h>
#include <ConversationEditor/ConversationEditorMainWindow.h>
#include <ConversationEditor/ConversationGraphContext.h>
#include <ConversationEditor/Nodes/ActorDialogue.h>
#include <ConversationEditor/Nodes/Link.h>
#include <ConversationEditor/Nodes/RootNode.h>
#include <ConversationEditor/Settings.h>

static constexpr const char* const ConversationEditorAppName = "Conversation Editor";

namespace Conversation
{
    void ConversationEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationEditorSystemComponent, ConversationSystemComponent>()->Version(0)->Attribute(
                AZ::Edit::Attributes::SystemComponentTags,
                AZStd::vector<AZ::Crc32>({
                    AssetBuilderSDK::ComponentTags::AssetBuilder,
                }));

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ConversationEditorSystemComponent>("ConversationEditor", "Handles editing conversation files.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }

        ConversationEditor::ConversationGraph::Reflect(context);
        ConversationEditor::ConversationDocument::Reflect(context);
        ConversationEditor::Nodes::Link::Reflect(context);
        ConversationEditor::Nodes::RootNode::Reflect(context);
        ConversationEditor::Nodes::ActorDialogue::Reflect(context);
        ConversationEditor::ConversationEditorSettings::Reflect(context);
    }

    ConversationEditorSystemComponent::ConversationEditorSystemComponent()
        : m_conversationEditorNodeManager(AZ_CRC_CE("ConversationEditor"))
    {
    }

    ConversationEditorSystemComponent::~ConversationEditorSystemComponent() = default;

    void ConversationEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("ConversationEditorService"));
    }

    void ConversationEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void ConversationEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void ConversationEditorSystemComponent::Init()
    {
        ConversationSystemComponent::Init();

        auto editorSettings = AZ::UserSettings::CreateFind<ConversationEditor::ConversationEditorSettings>(
            ConversationEditor::ConversationEditorSettingsId, AZ::UserSettings::CT_LOCAL);
    }

    void ConversationEditorSystemComponent::Activate()
    {
        ConversationSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void ConversationEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        ConversationSystemComponent::Deactivate();
    }

    void ConversationEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions options;
        options.paneRect = QRect(100, 100, 500, 400);
        options.showOnToolsToolbar = true;
        options.toolbarIcon = ":/Conversation/toolbar_icon.svg";
        options.isPreview = true;

        AzToolsFramework::EditorRequests::WidgetCreationFunc cb = [](QWidget*)
        {
            auto config = new ConversationEditor::ConversationAssetEditorWindowConfig();
            config->m_editorId = ConversationEditor::AssetEditorId;
            config->m_baseStyleSheet = "default_style.json";
            config->m_saveIdentifier = ConversationEditor::SAVE_IDENTIFIER;

            return new ConversationEditor::ConversationEditorMainWindow(config, nullptr);
        };

        // Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
        AzToolsFramework::RegisterViewPane<ConversationEditor::ConversationEditorMainWindow>(
            ConversationEditorAppName, "Conversation", options, cb);
    }

} // namespace Conversation
