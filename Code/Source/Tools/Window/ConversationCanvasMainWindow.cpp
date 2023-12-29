#include "Tools/Window/ConversationCanvasMainWindow.h"

#include "AtomToolsFramework/DynamicProperty/DynamicPropertyGroup.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportContent.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportInputController.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsInspector.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportToolBar.h"
#include "AtomToolsFramework/Graph/GraphDocumentRequestBus.h"
#include "AtomToolsFramework/Inspector/InspectorPropertyGroupWidget.h"
#include "AzQtComponents/Components/StyleManager.h"
#include "GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.h"
#include "GraphCanvas/Widgets/NodePalette/NodePaletteWidget.h"

#include "QApplication"
#include "QMessageBox"
#include "QWindow"
#include "qnamespace.h"

#include "Conversation/Constants.h"

namespace ConversationEditor
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
            new AtomToolsFramework::AtomToolsDocumentInspector(
                m_toolId, this); // NOLINT
        m_documentInspector->SetDocumentSettingsPrefix(
            ConversationCanvasSettingsDocumentInspectorKey);
        AddDockWidget(
            "Inspector", m_documentInspector, Qt::RightDockWidgetArea);

        m_toolBar = new AtomToolsFramework::EntityPreviewViewportToolBar(
            m_toolId, this); // NOLINT

        m_conversationViewport =
            new AtomToolsFramework::EntityPreviewViewportWidget(
                m_toolId, this); // NOLINT

        auto entityContext = AZStd::make_shared<AzFramework::EntityContext>();
        entityContext->InitContext();

        auto viewportScene =
            AZStd::make_shared<AtomToolsFramework::EntityPreviewViewportScene>(
                m_toolId,
                m_conversationViewport,
                entityContext,
                "ConversationCanvasViewportWidget",
                "passes/lowendrenderpipeline.azasset");

        auto viewportContent = AZStd::make_shared<
            AtomToolsFramework::EntityPreviewViewportContent>(
            m_toolId, m_conversationViewport, entityContext);

        auto viewportController = AZStd::make_shared<
            AtomToolsFramework::EntityPreviewViewportInputController>(
            m_toolId, m_conversationViewport, viewportContent);

        m_conversationViewport->Init(
            entityContext, viewportScene, viewportContent, viewportController);

        auto* viewPortAndToolbar = new QWidget(this); // NOLINT
        viewPortAndToolbar->setLayout(
            new QVBoxLayout(viewPortAndToolbar)); // NOLINT
        viewPortAndToolbar->layout()->setContentsMargins(0, 0, 0, 0);
        viewPortAndToolbar->layout()->setMargin(0);
        viewPortAndToolbar->layout()->setSpacing(0);
        viewPortAndToolbar->layout()->addWidget(m_toolBar);
        viewPortAndToolbar->layout()->addWidget(m_conversationViewport);

        AddDockWidget("Viewport", viewPortAndToolbar, Qt::BottomDockWidgetArea);

        m_viewportSettingsInspector =
            new AtomToolsFramework::EntityPreviewViewportSettingsInspector(
                m_toolId, this);
        AddDockWidget(
            "Viewport Settings",
            m_viewportSettingsInspector,
            Qt::LeftDockWidgetArea);
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
        AZ::Uuid const& documentId)
    {
        Base::OnDocumentOpened(documentId);
        m_documentInspector ? m_documentInspector->SetDocumentId(documentId)
                            : void();

        // Disconnect to ensure we aren't currently connected to anything, since
        // we'll be called multiple times.
        GraphCanvas::SceneNotificationBus::Handler::BusDisconnect();

        GraphCanvas::GraphId const openedDocumentGraphId =
            [&documentId]() -> auto
        {
            GraphCanvas::GraphId result{};
            AtomToolsFramework::GraphDocumentRequestBus::EventResult(
                result,
                documentId,
                &AtomToolsFramework::GraphDocumentRequests::GetGraphId);
            return result;
        }();

        if (openedDocumentGraphId.IsValid())
        {
            // WARNING: We must be connected to GraphCanvas'
            // SceneNotificationBus so OnPreNodeDeleted gets called.
            GraphCanvas::SceneNotificationBus::Handler::BusConnect(
                openedDocumentGraphId);
        }
    }

    void ConversationCanvasMainWindow::ResizeViewportRenderTarget(
        AZ::u32 width, AZ::u32 height)
    {
        QSize requestedViewportSize =
            QSize(width, height) / devicePixelRatioF();
        QSize currentViewportSize = m_conversationViewport->size();
        QSize offset = requestedViewportSize - currentViewportSize;
        QSize requestedWindowSize = size() + offset;
        resize(requestedWindowSize);

        AZ_Assert(
            m_conversationViewport->size() == requestedViewportSize,
            "Resizing the window did not give the expected viewport size. "
            "Requested %d x %d but got %d x %d.",
            requestedViewportSize.width(),
            requestedViewportSize.height(),
            m_conversationViewport->size().width(),
            m_conversationViewport->size().height());

        [[maybe_unused]] QSize newDeviceSize = m_conversationViewport->size();
        AZ_Warning(
            "Conversation Canvas",
            static_cast<uint32_t>(newDeviceSize.width()) == width &&
                static_cast<uint32_t>(newDeviceSize.height()) == height,
            "Resizing the window did not give the expected frame size. "
            "Requested %d x %d but got %d x %d.",
            width,
            height,
            newDeviceSize.width(),
            newDeviceSize.height());
    }

    void ConversationCanvasMainWindow::LockViewportRenderTargetSize(
        AZ::u32 width, AZ::u32 height)
    {
        m_conversationViewport->LockRenderTargetSize(width, height);
    }

    void ConversationCanvasMainWindow::UnlockViewportRenderTargetSize()
    {
        m_conversationViewport->UnlockRenderTargetSize();
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
                        ConversationCanvasSettingsEnablePreviewKey,
                        "Enable Preview Functionality",
                        "Just testing.",
                        false),
                    AtomToolsFramework::CreateSettingsPropertyValue(
                        ConversationCanvasSettingsForceDeleteGeneratedFilesKey,
                        "Force Delete Generated Files",
                        "Delete all files in generated folder after compiling.",
                        false),

                });

        inspector->AddGroup(
            m_conversationCanvasCompileSettingsGroup->m_name,
            m_conversationCanvasCompileSettingsGroup->m_displayName,
            m_conversationCanvasCompileSettingsGroup->m_description,
            aznew AtomToolsFramework::InspectorPropertyGroupWidget(
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
        return R"(A basic conversation/dialogue system for O3DE.)";
    }

    void ConversationCanvasMainWindow::OnPreNodeDeleted(
        AZ::EntityId const& nodeId)
    {
        // WARNING: Without reseting the inspector prior deleting a group node,
        // such as comments, the application will crash because the inspector is
        // doing something with memory that's just been deleted. It doesn't
        // happen with other node types. I have not been able to find the source
        // of the bug, but it happens with O3DE's own AtomTools applications as
        // well.
        m_documentInspector->Reset();
    }

} // namespace ConversationEditor

#include <Tools/Window/moc_ConversationCanvasMainWindow.cpp>
