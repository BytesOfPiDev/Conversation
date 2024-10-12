#include "Window/ConversationCanvasMainWindow.h"

#include "AtomToolsFramework/DynamicProperty/DynamicPropertyGroup.h"
#include "AtomToolsFramework/Inspector/InspectorPropertyGroupWidget.h"
#include "AzCore/IO/FileIO.h"
#include "AzQtComponents/Components/StyleManager.h"
#include "GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.h"
#include "GraphCanvas/Widgets/NodePalette/NodePaletteWidget.h"

#include "Common.h"

namespace ConversationCanvas
{
    using GraphCanvas::NodePaletteConfig;

    ConversationCanvasMainWindow::ConversationCanvasMainWindow(
        AZ::Crc32 const& toolId,
        AtomToolsFramework::GraphViewSettingsPtr const& graphViewSettingsPtr,
        QWidget* parent)
        : Base(toolId, "ConversationCanvasMainWindow", parent)
        , m_graphViewSettingsPtr(graphViewSettingsPtr)
        , m_styleManager(toolId, graphViewSettingsPtr->m_styleManagerPath)
    {
        m_documentInspector =
            new AtomToolsFramework::AtomToolsDocumentInspector(m_toolId, this);
        m_documentInspector->SetDocumentSettingsPrefix(
            ConversationCanvasSettingsDocumentInspectorKey);
        AddDockWidget(
            "Inspector", m_documentInspector, Qt::RightDockWidgetArea);

        SetDockWidgetVisible("Viewport Settings", false);

        m_bookmarkDockWidget =
            aznew GraphCanvas::BookmarkDockWidget(m_toolId, this);
        AddDockWidget(
            "Bookmarks", m_bookmarkDockWidget, Qt::BottomDockWidgetArea);
        SetDockWidgetVisible("Bookmarks", false);

        AddDockWidget(
            "MiniMap",
            aznew GraphCanvas::MiniMapDockWidget(m_toolId, this),
            Qt::BottomDockWidgetArea);
        SetDockWidgetVisible("MiniMap", false);

        NodePaletteConfig nodePaletteConfig;
        nodePaletteConfig.m_rootTreeItem =
            m_graphViewSettingsPtr->m_createNodeTreeItemsFn(m_toolId);
        nodePaletteConfig.m_editorId = m_toolId;
        nodePaletteConfig.m_mimeType =
            m_graphViewSettingsPtr->m_nodeMimeType.c_str();
        nodePaletteConfig.m_isInContextMenu = false;
        nodePaletteConfig.m_saveIdentifier =
            m_graphViewSettingsPtr->m_nodeSaveIdentifier;

        m_nodePalette = aznew GraphCanvas::NodePaletteDockWidget(
            this, "Node Palette", nodePaletteConfig);
        AddDockWidget("Node Palette", m_nodePalette, Qt::LeftDockWidgetArea);

        AZ::IO::FixedMaxPath resolvedPath;
        AZ::IO::FileIOBase::GetInstance()->ReplaceAlias(
            resolvedPath, m_graphViewSettingsPtr->m_translationPath.c_str());
        const AZ::IO::FixedMaxPathString translationFilePath =
            resolvedPath.LexicallyNormal().FixedMaxPathString();
        if (m_translator.load(
                QLocale::Language::English, translationFilePath.c_str()))
        {
            if (!qApp->installTranslator(&m_translator))
            {
                AZ_Warning(
                    "ConversationCanvas",
                    false,
                    "Error installing translation %s!",
                    translationFilePath.c_str());
            }
        }
        else
        {
            AZ_Warning(
                "ConversationCanvas",
                false,
                "Error loading translation file %s",
                translationFilePath.c_str());
        }

        // Set up style sheet to fix highlighting in the node palette
        AzQtComponents::StyleManager::setStyleSheet(
            this, QStringLiteral(":/GraphView/GraphView.qss"));

        OnDocumentOpened(AZ::Uuid::CreateNull());
    }

    void ConversationCanvasMainWindow::OnDocumentOpened(
        const AZ::Uuid& documentId)
    {
        Base::OnDocumentOpened(documentId);
        m_documentInspector->SetDocumentId(documentId);
    }

    void ConversationCanvasMainWindow::PopulateSettingsInspector(
        AtomToolsFramework::InspectorWidget* inspector) const
    {
        m_conversationCanvasCompileSettingsGroup =
            AtomToolsFramework::CreateSettingsPropertyGroup(
                "Conversation Canvas Settings",
                "Conversation Canvas Settings",
                {
                    AtomToolsFramework::CreateSettingsPropertyValue(
                        "/O3DE/AtomToolsFramework/GraphCompiler/EnableLogging",
                        "Enable Compiler Logging",
                        "Toggle verbose logging for material graph generation.",
                        false),
                    AtomToolsFramework::CreateSettingsPropertyValue(
                        Settings::FormatLua,
                        "Run stylua (experimental)",
                        "If true, stylua is run on @devassets@/Conversions to "
                        "format the lua files",
                        false),
                    AtomToolsFramework::CreateSettingsPropertyValue(
                        Settings::LuaFormatter,
                        "Path to stylua",
                        "The full path, including the executable name, to the "
                        "stylua",
                        AZStd::string{ "stylua" }),
                    AtomToolsFramework::CreateSettingsPropertyValue(
                        ConversationCanvasSettingsForceDeleteGeneratedFilesKey,
                        "Force Delete Generated Files",
                        "Delete all files in generated folder after compiling.",
                        ""),

                });

        inspector->AddGroup(
            m_conversationCanvasCompileSettingsGroup->m_name,
            m_conversationCanvasCompileSettingsGroup->m_displayName,
            m_conversationCanvasCompileSettingsGroup->m_description,
            new AtomToolsFramework::InspectorPropertyGroupWidget(
                m_conversationCanvasCompileSettingsGroup.get(),
                m_conversationCanvasCompileSettingsGroup.get(),
                azrtti_typeid<AtomToolsFramework::DynamicPropertyGroup>()));

        inspector->AddGroup(
            "Graph View Settings",
            "Graph View Settings",
            "Configuration settings for the graph view interaction, animation, "
            "and other behavior.",
            aznew AtomToolsFramework::InspectorPropertyGroupWidget(
                m_graphViewSettingsPtr.get(),
                m_graphViewSettingsPtr.get(),
                m_graphViewSettingsPtr->RTTI_Type()));

        Base::PopulateSettingsInspector(inspector);
    }

    void ConversationCanvasMainWindow::OnSettingsDialogClosed()
    {
        AtomToolsFramework::SetSettingsObject(
            ConversationCanvasGraphViewSettingsKey, m_graphViewSettingsPtr);
        Base::OnSettingsDialogClosed();
    }

    auto ConversationCanvasMainWindow::GetHelpDialogText() const
        -> AZStd::string
    {
        return R"(A dialogue system for O3DE.)";
    }

    void ConversationCanvasMainWindow::OnPreNodeDeleted(
        AZ::EntityId const& /*nodeId*/)
    {
        // WARNING: Without reseting the inspector prior deleting a group node,
        // such as comments, the application will crash because the inspector is
        // doing something with memory that's just been deleted. It doesn't
        // happen with other node types. I have not been able to find the source
        // of the bug, but it happens with O3DE's own AtomTools applications as
        // well.
        m_documentInspector->Reset();
    }

} // namespace ConversationCanvas

#include <Window/moc_ConversationCanvasMainWindow.cpp>
