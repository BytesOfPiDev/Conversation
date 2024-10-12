/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "ConversationCanvasApplication.h"

#include "Atom/RPI.Reflect/Image/StreamingImageAsset.h"
#include "AtomToolsFramework/Document/AtomToolsAnyDocument.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentApplication.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentSystemRequestBus.h"
#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeUtil.h"
#include "AtomToolsFramework/Graph/GraphDocument.h"
#include "AtomToolsFramework/Graph/GraphDocumentView.h"
#include "AtomToolsFramework/Util/Util.h"
#include "AzCore/Math/Color.h"
#include "AzCore/Math/Vector2.h"
#include "AzCore/Math/Vector3.h"
#include "AzCore/Math/Vector4.h"
#include "AzCore/RTTI/RTTI.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "GraphModel/Integration/GraphControllerManager.h"
#include "GraphModel/Model/DataType.h"

#include "Common.h"
#include "Conversation/DialogueChunk.h"
#include "Conversation/UniqueId.h"
#include "ConversationGraphContext.h"
#include "DataTypes.h"
#include "Document/ConversationGraphCompiler.h"
#include "Document/NodeRequestBus.h"
#include "GraphModel/Integration/NodePalette/StandardNodePaletteItem.h"
#include "Window/ConversationCanvasMainWindow.h"
#include "Window/Nodes/Link.h"

#include <QLabel>

void InitConversationCanvasResources()
{
    // Must register qt resources from other modules
    Q_INIT_RESOURCE(ConversationCanvas);
    Q_INIT_RESOURCE(InspectorWidget);
    Q_INIT_RESOURCE(AtomToolsAssetBrowser);
    Q_INIT_RESOURCE(GraphView);
}

namespace ConversationCanvas
{
    static char const* GetBuildTargetName()
    {
#if !defined(LY_CMAKE_TARGET)
#error                                                                         \
    "LY_CMAKE_TARGET must be defined in order to add this source file to a CMake executable target"
#endif
        return LY_CMAKE_TARGET;
    }

    ConversationCanvasApplication::ConversationCanvasApplication(
        int* argc, char*** argv)
        : Base(GetBuildTargetName(), argc, argv)
    {
        InitConversationCanvasResources();

        QApplication::setOrganizationName("Bytes of Pi");
        QApplication::setApplicationName("O3DE Conversation Canvas");
        QApplication::setWindowIcon(QIcon(":/Icons/application.svg"));

        AzToolsFramework::EditorWindowRequestBus::Handler::BusConnect();
        AZ::RHI::FactoryManagerNotificationBus::Handler::BusConnect();
        AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler::
            BusConnect(m_toolId);
    }

    ConversationCanvasApplication::~ConversationCanvasApplication()
    {
        AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler::
            BusDisconnect(m_toolId);
        AzToolsFramework::EditorWindowRequestBus::Handler::BusDisconnect();
        AZ::RHI::FactoryManagerNotificationBus::Handler::BusDisconnect();
        m_window.reset();
    }

    void ConversationCanvasApplication::Reflect(AZ::ReflectContext* context)
    {
        Base::Reflect(context);
        ConversationGraphCompiler::Reflect(context);
        GraphModelIntegration::ReflectAndCreateNodeMimeEvent<LinkNode>(context);

        if (auto serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->RegisterGenericType<AZStd::array<AZ::Vector2, 2>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector3, 3>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector4, 3>>();
            serialize->RegisterGenericType<AZStd::array<AZ::Vector4, 4>>();

            serialize->Class<NodeRequests>()->Version(0);
        }
    }

    char const* ConversationCanvasApplication::GetCurrentConfigurationName()
        const
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
        m_graphContext.reset();
        m_graphTemplateFileDataCache.reset();
        m_dynamicNodeManager.reset();

        Base::Destroy();
    }

    AZStd::vector<AZStd::string> ConversationCanvasApplication::
        GetCriticalAssetFilters() const
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
                AZ_CRC_CE("float2"), AZ::Vector2{}, "float2"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float3"), AZ::Vector3{}, "float3"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float4"), AZ::Vector4{}, "float4"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float2x2"),
                AZStd::array<AZ::Vector2, 2>{ AZ::Vector2(1.0f, 0.0f),
                                              AZ::Vector2(0.0f, 1.0f) },
                "float2x2"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float3x3"),
                AZStd::array<AZ::Vector3, 3>{ AZ::Vector3(1.0f, 0.0f, 0.0f),
                                              AZ::Vector3(0.0f, 1.0f, 0.0f),
                                              AZ::Vector3(0.0f, 0.0f, 1.0f) },
                "float3x3"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float4x3"),
                AZStd::array<AZ::Vector4, 3>{
                    AZ::Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                    AZ::Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                    AZ::Vector4(0.0f, 0.0f, 1.0f, 0.0f) },
                "float4x3"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("float4x4"),
                AZStd::array<AZ::Vector4, 4>{
                    AZ::Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                    AZ::Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                    AZ::Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                    AZ::Vector4(0.0f, 0.0f, 0.0f, 1.0f) },
                "float4x4"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("color"), AZ::Color::CreateOne(), "color"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("string"), AZStd::string{}, "string"),
            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("image"),
                AZ::Data::Asset<AZ::RPI::StreamingImageAsset>{
                    AZ::Data::AssetLoadBehavior::NoLoad },
                "image"),

            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("string"), AZStd::string{}, "string"),

            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::actor_text),
                AZStd::string{},
                ToString(SlotTypes::actor_text)),
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
                AZ_CRC_CE("unique_id"),
                Conversation::UniqueId::TYPEINFO_Uuid(),
                AZStd::make_any<Conversation::UniqueId>(),
                Conversation::UniqueId::TYPEINFO_Name(),
                Conversation::UniqueId::TYPEINFO_Name()),

            AZStd::make_shared<GraphModel::DataType>(
                AZ_CRC_CE("crc32"), AZ::Crc32{}, "crc32"),

            AZStd::make_shared<GraphModel::DataType>(
                ToTag(SlotTypes::audio_control),
                Conversation::DialogueAudioControl{},
                ToString(SlotTypes::audio_control)),

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
            "conversationPropertyMember", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyDescription", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conditionInstructions", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::LineEdit;
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyName", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyDisplayName", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyConnectionName", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyGroupName", editData);
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyGroup", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ::Edit::Attributes::StringList,
            AZStd::vector<AZStd::string>{ "", "0", "1", "2", "3", "4" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyMinVectorSize", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ::Edit::Attributes::StringList,
            AZStd::vector<AZStd::string>{ "None",
                                          "ScriptInput",
                                          "ScriptOption",
                                          "ScriptEnabled",
                                          "InternalProperty",
                                          "" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "conversationPropertyConnectionType", editData);

        editData = {};
        editData.m_elementId = AZ_CRC_CE("StringFilePath");
        AtomToolsFramework::AddEditDataAttribute(
            editData, AZ_CRC_CE("Title"), AZStd::string("Template File"));
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ_CRC_CE("Extensions"),
            AZStd::vector<AZStd::string>{
                "lua",
                "conversationasset",
            });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "templatePaths", editData);

        editData = {};
        editData.m_elementId = AZ_CRC_CE("StringFilePath");
        AtomToolsFramework::AddEditDataAttribute(
            editData, AZ_CRC_CE("Title"), AZStd::string("Include File"));
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ_CRC_CE("Extensions"),
            AZStd::vector<AZStd::string>{ "azsli" });
        m_dynamicNodeManager->RegisterEditDataForSetting(
            "includePaths", editData);

        editData = {};
        editData.m_elementId = AZ::Edit::UIHandlers::ComboBox;
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ::Edit::Attributes::StringList,
            AZStd::vector<AZStd::string>{
                "Dialogue", "Condition", "Function", "Snippet" });
        m_dynamicNodeManager->RegisterEditDataForSetting("nodeType", editData);

        editData = {};
        editData.m_elementId = AZ_CRC_CE("StringFilePath");
        AtomToolsFramework::AddEditDataAttribute(
            editData, AZ_CRC_CE("Title"), AZStd::string("Template File"));
        AtomToolsFramework::AddEditDataAttribute(
            editData,
            AZ_CRC_CE("Extensions"),
            AZStd::vector<AZStd::string>{ "lua", "conversationtemplate" });
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
        m_graphContext = AZStd::make_shared<ConversationGraphContext>(
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

            auto* const coreNode =
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
            AtomToolsFramework::GetSettingsObject(
                "/O3DE/Atom/GraphView/DefaultGroupPresets",
                AZStd::map<AZStd::string, AZ::Color>{
                    { "Logic", AZ::Color(0.188f, 0.972f, 0.243f, 1.0f) },
                    { "Function", AZ::Color(0.396f, 0.788f, 0.788f, 1.0f) },
                    { "Output", AZ::Color(0.866f, 0.498f, 0.427f, 1.0f) },
                    { "Input", AZ::Color(0.396f, 0.788f, 0.549f, 1.0f) } });

        // Connect the graph view settings to the required buses so that they
        // can be accessed throughout the application.
        m_graphViewSettingsPtr->Initialize(m_toolId, defaultGroupPresets);
    }

    void ConversationCanvasApplication::InitConversationGraphDocumentType()
    {
        // Initialize system to asynchronously report conversation and script
        // related asset processing status for open documents
        m_assetStatusReporterSystem =
            AZStd::make_unique<AtomToolsFramework::AssetStatusReporterSystem>(
                m_toolId);

        // Initialize system to load and store conversation graph template files
        // and only reload them if modified
        m_graphTemplateFileDataCache =
            AZStd::make_unique<AtomToolsFramework::GraphTemplateFileDataCache>(
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
                "the "
                "inspector.",
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

    void ConversationCanvasApplication::InitMainWindow()
    {
        m_window.reset(aznew ConversationCanvasMainWindow(
            m_toolId, m_graphViewSettingsPtr));
        m_window->show();
    }

    void ConversationCanvasApplication::InitDefaultDocument()
    {
        // Create an untitled, empty graph document as soon as the application
        // starts so the user can begin creating immediately.
        if (AtomToolsFramework::GetSettingsValue(
                ConversationCanvasSettingsCreateDefaultDocumentOnStartKey,
                false))
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

    AZ::ComponentTypeList ConversationCanvasApplication::
        GetRequiredSystemComponents() const
    {
        auto requiredSystemComponents{
            decltype(AtomToolsFramework::AtomToolsDocumentApplication::
                         GetRequiredSystemComponents()){}
        };

        requiredSystemComponents.emplace_back(
            AZ::TypeId{ "{58CE2D43-2DDC-4CEB-BB9F-61B77C50C35D}" });

        auto baseReqs = AtomToolsFramework::AtomToolsDocumentApplication::
            GetRequiredSystemComponents();

        requiredSystemComponents.insert(
            requiredSystemComponents.end(), baseReqs.begin(), baseReqs.end());

        return requiredSystemComponents;
    }
} // namespace ConversationCanvas
