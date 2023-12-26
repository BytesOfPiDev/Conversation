#include "Tools/ConversationCanvasApplication.h"

#include "AtomToolsFramework/Document/AtomToolsAnyDocument.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentNotificationBus.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentSystemRequestBus.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsSystem.h"
#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeUtil.h"
#include "AtomToolsFramework/Graph/GraphDocument.h"
#include "AtomToolsFramework/Graph/GraphDocumentView.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Math/Color.h"
#include "AzCore/Math/Vector2.h"
#include "AzCore/Math/Vector3.h"
#include "AzCore/Math/Vector4.h"
#include "AzCore/RTTI/RTTI.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptAsset.h"
#include "Conversation/DialogueData.h"
#include "Conversation/UniqueId.h"
#include "GraphModel/Integration/NodePalette/StandardNodePaletteItem.h"
#include "GraphModel/Model/DataType.h"

#include "Conversation/Constants.h"
#include "Tools/ConversationGraphContext.h"
#include "Tools/DataTypes.h"
#include "Tools/Document/ConversationGraphCompiler.h"
#include "Tools/Window/ConversationCanvasMainWindow.h"

#include "QLabel"
#include "Tools/Window/Nodes/Link.h"

void InitConversationCanvasResources()
{
    // Must register qt resources from other modules
    Q_INIT_RESOURCE(ConversationCanvas);
    Q_INIT_RESOURCE(InspectorWidget);
    Q_INIT_RESOURCE(AtomToolsAssetBrowser);
    Q_INIT_RESOURCE(GraphView);
}

namespace ConversationEditor
{

    static auto GetBuildTargetName() -> char const*
    {
#if !defined(LY_CMAKE_TARGET)
#error                                                                         \
    "LY_CMAKE_TARGET must be defined in order to add this source file to a cmake executable target."
#endif
        return LY_CMAKE_TARGET;
    }

    ConversationCanvasApplication::ConversationCanvasApplication(
        int* argc, char*** argv)
        : Base(GetBuildTargetName(), argc, argv)
    {
        InitConversationCanvasResources();

        QApplication::setOrganizationName("Bytes of Pi");
        QApplication::setApplicationName(
            "O3DE Conversation Canvas (prototype)");
        QApplication::setWindowIcon(
            QIcon(":/ConversationCanvas/toolbar_icon.svg"));

        AzToolsFramework::EditorWindowRequestBus::Handler::BusConnect();
        AZ::RHI::FactoryManagerNotificationBus::Handler::BusConnect();
        AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler::
            BusConnect(m_toolId);
    }

    ConversationCanvasApplication::~ConversationCanvasApplication()
    {
        AzToolsFramework::EditorWindowRequestBus::Handler::BusDisconnect();
        AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler::
            BusDisconnect(m_toolId);
        AZ::RHI::FactoryManagerNotificationBus::Handler::BusDisconnect();

        m_window.reset();
    }

    void ConversationCanvasApplication::Reflect(AZ::ReflectContext* context)
    {
        Base::Reflect(context);
        ConversationGraphCompiler::Reflect(context);

        GraphModelIntegration::ReflectAndCreateNodeMimeEvent<LinkNode>(context);

        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->RegisterGenericType<AZStd::array<AZ::Vector2, 2>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector3, 3>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector4, 3>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector4, 4>>();

            serialize->Class<NodeRequests>()->Version(0);
        }
    }

    auto ConversationCanvasApplication::GetCurrentConfigurationName() const
        -> char const*
    {
#if defined(_RELEASE)
        return "ReleaseConversationCanvas";
#elif defined(_DEBUG)
        return "DebugConversationCanvas";
#else
        return "ProfileConversationCanvas";
#endif
    }

    void ConversationCanvasApplication::StartCommon(AZ::Entity* systemEntity)
    {
        Base::StartCommon(systemEntity);

        InitDynamicNodeManager();
        InitDynamicNodeEditData();
        InitSharedGraphContext();
        InitGraphViewSettings();
        InitConversationGraphDocumentType();
        InitConversationGraphNodeDocumentType();
        InitDialogueSourceDataDocumentType();
        InitMainWindow();
        InitDefaultDocument();
    }

    void ConversationCanvasApplication::Destroy()
    {
        // Save all of the graph view configuration settings to the settings
        // registry.
        AtomToolsFramework::SetSettingsObject(
            ConversationCanvasGraphViewSettingsKey, m_graphViewSettingsPtr);

        m_graphViewSettingsPtr.reset();
        m_window.reset();
        m_viewportSettingsSystem.reset();
        m_graphContext.reset();
        m_graphTemplateFileDataCache.reset();
        m_dynamicNodeManager.reset();

        Base::Destroy();
    }

    auto ConversationCanvasApplication::GetCriticalAssetFilters() const
        -> AZStd::vector<AZStd::string>
    {
        return AZStd::vector<AZStd::string>(
            { "passes/", "config/", "ConversationCanvas/" });
    }

    auto ConversationCanvasApplication::GetAppMainWindow() -> QWidget*
    {
        return m_window.get();
    }

    void ConversationCanvasApplication::FactoryRegistered()
    {
    }

    void ConversationCanvasApplication::InitDynamicNodeManager()
    {
        // Instantiate the dynamic node manager to register all dynamic node
        // configurations and data types used in this tool
        m_dynamicNodeManager =
            AZStd::make_unique<AtomToolsFramework::DynamicNodeManager>(
                m_toolId);

        // Register all data types required by Conversation Canvas nodes with
        // the dynamic node manager
        m_dynamicNodeManager->RegisterDataTypes({
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("bool"), bool{}, "bool"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("int"), int32_t{}, "int"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("uint"), uint32_t{}, "uint"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float"), float{}, "float"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("string"), AZStd::string{}, "string"),

            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::actor_text),
                AZStd::string{},
                ToString(SlotTypes::actor_text)),
            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::dialogue_script),
                AZStd::string{},
                ToString(SlotTypes::dialogue_script)),
            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::speaker_tag),
                AZStd::string{},
                ToString(SlotTypes::speaker_tag)),
            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::lua_snippet),
                Conversation::DialogueChunk{},
                ToString(SlotTypes::lua_snippet)),
            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::dialogue_chunk),
                Conversation::DialogueChunk{},
                ToString(SlotTypes::dialogue_chunk)),
            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::dialogue_id),
                Conversation::UniqueId{},
                ToString(SlotTypes::dialogue_id)),

            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::condition_snippet),
                AZStd::make_any<AZStd::string>(),
                ToString(SlotTypes::condition_snippet)),

            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("unique_id"),
                Conversation::UniqueId::TYPEINFO_Uuid(),
                AZStd::make_any<Conversation::UniqueId>(),
                Conversation::UniqueId::TYPEINFO_Name(),
                Conversation::UniqueId::TYPEINFO_Name()),

            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("crc32"), AZ::Crc32{}, "crc32"),
        });

        // Search the project and gems for dynamic node configurations and
        // register them with the manager
        m_dynamicNodeManager->LoadConfigFiles("conversationgraphnode");
    }

    void ConversationCanvasApplication::InitDynamicNodeEditData()
    {
        // Registering custom property handlers for dynamic node configuration
        // settings. The settings are just a map of string data. Recognized
        // settings will need special controls for selecting files or editing
        // large blocks of text without taking up much real estate in the
        // property editor. In the future, this will likely be replaced with a
        // more specialized node configuration editor.
        AZ::Edit::ElementData editData;
        editData.m_elementId = AZ_CRC_CE("MultilineStringDialog");
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "instructions", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conditionInstructions", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "classDefinitions", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "functionDefinitions", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conditionInstructions", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ::Edit::Attributes::StringList,
            AZStd::vector<AZStd::string>{ "==", "!=", ">=", "<=", "" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conditionFunctionOp", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ::Edit::Attributes::StringList,
            AZStd::vector<AZStd::string>{ "Dialogue", "Condition", "Script" });
        m_dynamicNodeManager->RegisterEditDataForSetting("nodeType", editData);

        editData = {};
        editData.m_elementId = AZ_CRC_CE("StringFilePath");
        AtomToolsFramework::AddEditDataAttribute(
            editData, AZ_CRC_CE("Title"), AZStd::string("Template File"));
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ_CRC_CE("Extensions"),
            AZStd::vector<AZStd::string>{
                "lua", "dlgcondition", "conversationtemplate" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "templatePaths", editData);

        editData = {};
        editData.m_elementId = AZ_CRC_CE("StringFilePath");
        AtomToolsFramework::AddEditDataAttribute(
            editData, AZ_CRC_CE("Title"), AZStd::string("Include File"));
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ_CRC_CE("Extensions"),
            AZStd::vector<AZStd::string>{ "lua" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "includePaths", editData);
    }

    void ConversationCanvasApplication::InitSharedGraphContext()
    {
        // Each graph document creates its own graph context but we want to use
        // a shared graph context instead to avoid data duplication
        m_graphContext =
            AZStd::make_shared<ConversationEditor::ConversationGraphContext>(
                m_dynamicNodeManager->GetRegisteredDataTypes());

        m_graphContext->CreateModuleGraphManager();
    }

    void ConversationCanvasApplication::InitGraphViewSettings()
    {
        // This configuration data is passed through the main window and graph
        // views to setup translation data, styling, and node palettes
        m_graphViewSettingsPtr = AtomToolsFramework::GetSettingsObject(
            ConversationCanvasGraphViewSettingsKey,
            AZStd::make_shared<AtomToolsFramework::GraphViewSettings>());

        // Initialize the application specific graph view settings that are not
        // serialized.
        m_graphViewSettingsPtr->m_translationPath =
            "@products@/conversationcanvas/translation/"
            "conversationcanvas_en_us.qm";
        m_graphViewSettingsPtr->m_styleManagerPath =
            "ConversationCanvas/StyleSheet/conversationcanvas_style.json";
        m_graphViewSettingsPtr->m_nodeMimeType =
            "ConversationCanvas/node-palette-mime-event";
        m_graphViewSettingsPtr->m_nodeSaveIdentifier =
            "ConversationCanvas/ContextMenu";
        m_graphViewSettingsPtr->m_createNodeTreeItemsFn =
            [](const AZ::Crc32& toolId)
        {
            GraphCanvas::GraphCanvasTreeItem* rootTreeItem = {};
            AtomToolsFramework::DynamicNodeManagerRequestBus::EventResult(
                rootTreeItem,
                toolId,
                &AtomToolsFramework::DynamicNodeManagerRequestBus::Events::
                    CreateNodePaletteTree);

            auto coreNode =
                rootTreeItem->CreateChildNode<GraphCanvas::NodePaletteTreeItem>(
                    "Core", toolId);

            coreNode->CreateChildNode<
                GraphModelIntegration::StandardNodePaletteItem<LinkNode>>(
                "Link", toolId);

            return rootTreeItem;
        };

        // Initialize the default group preset names and colors needed by the
        // graph canvas view to create node groups.
        AZStd::map<AZStd::string, AZ::Color> const defaultGroupPresets =
            AZStd::map<AZStd::string, AZ::Color>{
                { "Logic", AZ::Color(0.188f, 0.972f, 0.243f, 1.0f) },
                { "Function", AZ::Color(0.396f, 0.788f, 0.788f, 1.0f) },
                { "Output", AZ::Color(0.866f, 0.498f, 0.427f, 1.0f) },
                { "Input", AZ::Color(0.396f, 0.788f, 0.549f, 1.0f) }
            };

        // Connect the graph view settings to the required buses so that they
        // can be accessed throughout the application.
        m_graphViewSettingsPtr->Initialize(m_toolId, defaultGroupPresets);
    }
    void ConversationCanvasApplication::InitConversationGraphDocumentType()
    {
        m_graphTemplateFileDataCache =
            AZStd::make_shared<AtomToolsFramework::GraphTemplateFileDataCache>(
                m_toolId);

        // Acquiring default Conversation Canvas document type info so that it
        // can be customized before registration
        auto documentTypeInfo =
            AtomToolsFramework::GraphDocument::BuildDocumentTypeInfo(
                "Conversation Graph",
                { "conversationgraph" },
                { "conversationgraphtemplate" },
                AtomToolsFramework::GetPathWithoutAlias(
                    AtomToolsFramework::GetSettingsValue<AZStd::string>(
                        "/O3DE/Atom/ConversationCanvas/"
                        "DefaultConversationGraphTemplate",
                        "@gemroot:ConversationCanvas@/Assets/"
                        "ConversationCanvas/GraphData/"
                        "blank_graph.conversationgraphtemplate")),
                m_graphContext,
                [toolId = m_toolId]()
                {
                    return AZStd::make_shared<ConversationGraphCompiler>(
                        toolId);
                });

        // Overriding documentview factory function to create graph view
        documentTypeInfo.m_documentViewFactoryCallback =
            [this](const AZ::Crc32& toolId, const AZ::Uuid& documentId)
        {
            m_window->AddDocumentTab(
                documentId,
                // NOLINTNEXTLINE(*-owning-memory)
                aznew AtomToolsFramework::GraphDocumentView(
                    toolId,
                    documentId,
                    m_graphViewSettingsPtr,
                    m_window.get()));
            return true;
        };

        AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Event(
            m_toolId,
            &AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Handler::
                RegisterDocumentType,
            documentTypeInfo);
    }

    void ConversationCanvasApplication::InitConversationGraphNodeDocumentType()
    {
        // Register document type for editing Conversation Canvas node
        // configurations. This document type does not have a central view
        // widget and will show a label directing users to the inspector.
        auto documentTypeInfo =
            AtomToolsFramework::AtomToolsAnyDocument::BuildDocumentTypeInfo(
                "Conversation Graph Node Config",
                { "conversationgraphnode" },
                { "conversationgraphnodetemplate" },
                AZStd::any(AtomToolsFramework::DynamicNodeConfig()),
                AZ::Uuid::CreateNull()); // Null ID because JSON file contains
                                         // type info and can be loaded directly
                                         // into AZStd::any

        documentTypeInfo.m_documentViewFactoryCallback =
            [this](
                [[maybe_unused]] const AZ::Crc32& toolId,
                const AZ::Uuid& documentId)
        {
            auto viewWidget = new QLabel(
                "Conversation Graph Node Config properties can be edited in "
                "the inspector.",
                m_window.get());
            viewWidget->setAlignment(Qt::AlignCenter);
            return m_window->AddDocumentTab(documentId, viewWidget);
        };

        AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Event(
            m_toolId,
            &AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Handler::
                RegisterDocumentType,
            documentTypeInfo);
    }
    void ConversationCanvasApplication::InitDialogueSourceDataDocumentType()
    {
    }
    void ConversationCanvasApplication::InitMainWindow()
    {
        m_viewportSettingsSystem = AZStd::make_unique<
            AtomToolsFramework::EntityPreviewViewportSettingsSystem>(m_toolId);

        m_window = AZStd::make_unique<ConversationCanvasMainWindow>(
            m_toolId, m_graphViewSettingsPtr);
        m_window->show();
    }
    void ConversationCanvasApplication::InitDefaultDocument()
    {
        // Create an untitled, empty graph document as soon as the application
        // starts so the user can begin creating immediately.
        if (AtomToolsFramework::GetSettingsValue(
                ConversationCanvasSettingsCreateDefaultDocumentOnStartKey,
                true))
        {
            AZ::Uuid documentId = AZ::Uuid::CreateNull();
            AtomToolsFramework::AtomToolsDocumentSystemRequestBus::EventResult(
                documentId,
                m_toolId,
                &AtomToolsFramework::AtomToolsDocumentSystemRequestBus::
                    Handler::CreateDocumentFromTypeName,
                "Conversation Graph");

            AtomToolsFramework::AtomToolsDocumentNotificationBus::Event(
                m_toolId,
                &AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler::
                    OnDocumentOpened,
                documentId);
        }
    }

} // namespace ConversationEditor
