#include <ConversationEditor/ConversationEditorMainWindow.h>

#include <AzCore/Serialization/Utils.h>
#include <AzQtComponents/Components/StyledDockWidget.h>
#include <Conversation/DialogueData.h>
#include <ConversationEditor/Common.h>
#include <ConversationEditor/ConversationDocument.h>
#include <ConversationEditor/DataTypes.h>
#include <ConversationEditor/Nodes/ActorDialogue.h>
#include <ConversationEditor/Nodes/Link.h>
#include <ConversationEditor/Nodes/RootNode.h>
#include <ConversationEditor/VariableNodePaletteTreeItemTypes.h>
#include <ConversationEditor/ui_ConversationEditorWidget.h>
#include <GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorDockWidget.h>
#include <GraphModel/GraphModelBus.h>
#include <GraphModel/Integration/NodePalette/GraphCanvasNodePaletteItems.h>
#include <GraphModel/Integration/NodePalette/InputOutputNodePaletteItem.h>
#include <GraphModel/Integration/NodePalette/StandardNodePaletteItem.h>
#include <QFileDialog>
#include <QMenuBar>

namespace ConversationEditor
{
    ConversationEditor::ConversationEditorMainWindow::ConversationEditorMainWindow(
        ConversationAssetEditorWindowConfig* config, QWidget* parent /*= nullptr*/)
        : GraphModelIntegration::EditorMainWindow(config, parent)
        , m_config(config)
        , m_rightDockWidget(nullptr)
        , m_graphContext(nullptr)
        , m_fileOpenAction(nullptr)
        , m_fileSaveAction(nullptr)
    {
        config ? m_graphContext = config->GetGraphContext() : m_graphContext = nullptr;
        SetupUI();
    }

    ConversationEditor::ConversationEditorMainWindow::~ConversationEditorMainWindow()
    {
        m_graphContext = nullptr;
    }

    GraphModel::GraphContextPtr ConversationEditor::ConversationEditorMainWindow::GetGraphContext() const
    {
        return m_graphContext;
    }

    void ConversationEditor::ConversationEditorMainWindow::OnEditorClosing(GraphCanvas::EditorDockWidget* dockWidget)
    {
        GraphModelIntegration::EditorMainWindow::OnEditorClosing(dockWidget);
    }

    void ConversationEditor::ConversationEditorMainWindow::OnEditorOpened(GraphCanvas::EditorDockWidget* dockWidget)
    {
        GraphCanvas::AssetEditorMainWindow::OnEditorOpened(dockWidget);

        GraphCanvas::GraphId graphId = dockWidget->GetGraphId();

        // Create the new graph.
        auto graph = AZStd::make_shared<ConversationGraph>(GetGraphContext());
        m_graphs[graphId] = graph;

        // Create the controller for the new graph.
        GraphModelIntegration::GraphManagerRequestBus::Broadcast(
            &GraphModelIntegration::GraphManagerRequests::CreateGraphController, graphId, graph);

        // Listen for GraphController notifications on the new graph.
        GraphModelIntegration::GraphControllerNotificationBus::MultiHandler::BusConnect(graphId);
    }

    void ConversationEditor::ConversationEditorMainWindow::OnWrapperNodeActionWidgetClicked(
        const AZ::EntityId& wrapperNode, const QRect& actionWidgetBoundingRect, const QPointF& scenePoint, const QPoint& screenPoint)
    {
        GraphModelIntegration::EditorMainWindow::OnWrapperNodeActionWidgetClicked(
            wrapperNode, actionWidgetBoundingRect, scenePoint, screenPoint);
    }

    GraphCanvas::GraphCanvasTreeItem* ConversationEditor::ConversationAssetEditorWindowConfig::CreateNodePaletteRoot()
    {
        auto root = aznew GraphModelIntegration::NodeGroupNodePaletteTreeItem("RootPaletteTreeItem", AssetEditorId);
        GraphModelIntegration::AddCommonNodePaletteUtilities(root, AssetEditorId);

        auto dialogueCreationTreeItem = aznew GraphModelIntegration::NodeGroupNodePaletteTreeItem("Dialogue Creation", AssetEditorId);

        AZ_UNUSED(dialogueCreationTreeItem);

        auto coreNode = root->CreateChildNode<VariableCategoryNodePaletteTreeItem>("Core");

        coreNode->CreateChildNode<GraphModelIntegration::StandardNodePaletteItem<Nodes::Link>>("Link", AssetEditorId);
        coreNode->CreateChildNode<GraphModelIntegration::StandardNodePaletteItem<Nodes::RootNode>>("Root", AssetEditorId);
        coreNode->CreateChildNode<GraphModelIntegration::StandardNodePaletteItem<Nodes::ActorDialogue>>("Actor Dialogue", AssetEditorId);

        return root;
    }

    void ConversationEditorMainWindow::SetupUI()
    {
        {
            m_rightDockWidget = new AzQtComponents::StyledDockWidget("Right Dock Widget", this);
            m_rightDockWidget->setObjectName(m_rightDockWidget->windowTitle());
            this->addDockWidget(Qt::RightDockWidgetArea, m_rightDockWidget);

            AZ::SerializeContext* serializeContext = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
        }

        m_bottomDockWidget = new AzQtComponents::StyledDockWidget("Bottom Dock Widget", this);
        m_bottomDockWidget->setObjectName(m_bottomDockWidget->windowTitle());

        this->addDockWidget(Qt::BottomDockWidgetArea, m_bottomDockWidget);

        Ui::ConversationEditorWidget editorui;
        QWidget* ConversationEditor = new QWidget;
        editorui.setupUi(ConversationEditor);
        m_bottomDockWidget->setWidget(ConversationEditor);

        m_actorTextEdit = editorui.dialogueEdit;

        QObject::connect(
            this, &ConversationEditorMainWindow::nodeSelectionChanged, this, &ConversationEditorMainWindow::OnNodeSelectionChanged);
        QObject::connect(m_actorTextEdit, &QPlainTextEdit::textChanged, this, &ConversationEditorMainWindow::OnActorTextChanged);
    }

    QAction* ConversationEditorMainWindow::AddFileOpenAction(QMenu* menu)
    {
        m_fileOpenAction = AZStd::shared_ptr<QAction>(EditorMainWindow::AddFileOpenAction(menu));

        QObject::connect(
            m_fileOpenAction.get(), &QAction::triggered,
            [this]
            {
                OnFileOpenTriggered();
            });

        menu->addAction(m_fileOpenAction.get());

        return m_fileOpenAction.get();
    }

    QAction* ConversationEditorMainWindow::AddFileSaveAction(QMenu* menu)
    {
        m_fileSaveAction = AZStd::shared_ptr<QAction>(EditorMainWindow::AddFileSaveAction(menu));

        QObject::connect(
            m_fileSaveAction.get(), &QAction::triggered,
            [this]
            {
                OnFileSaveTriggered();
            });

        menu->addAction(m_fileSaveAction.get());

        return m_fileSaveAction.get();
    }

    void ConversationEditorMainWindow::OnSelectionChanged()
    {
        GraphCanvas::AssetEditorMainWindow::OnSelectionChanged();

        // Find the node that is currently selected, if any.
        GraphModel::NodePtrList result;
        GraphModelIntegration::GraphControllerRequestBus::EventResult(
            result, this->GetActiveGraphCanvasGraphId(), &GraphModelIntegration::GraphControllerRequestBus::Events::GetSelectedNodes);

        // If no node is selected, we send the appropriate signals to all parts of the UI
        // can update as necessary. Typically this means disabling all widgets that allow
        // manipulation of a DialogueData object/node.
        if (result.empty())
        {
            emit nodeSelectionChanged(nullptr);
            return;
        }

        AZ_Assert(
            result.front()->RTTI_IsTypeOf(AZ::AzTypeInfo<GraphModel::Slot>::Uuid()),
            "Result is not a slot. Happens in o3de branches without PR#5206 merged.");

        const GraphModel::SlotPtr dialogueDataSlotPtr = result.front()->GetSlot(CommonSlotNames::DIALOGUEDATA);
        if (!dialogueDataSlotPtr)
        {
            emit nodeSelectionChanged(nullptr);
            return;
        }

        const auto dialogueDataPtr = dialogueDataSlotPtr->GetValue<Conversation::DialogueDataPtr>();
        AZ_Assert(dialogueDataPtr != nullptr, "DialogueData retrieved from slot is null.");

        emit nodeSelectionChanged(dialogueDataPtr);
    }

    void ConversationEditorMainWindow::OnNodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr)
    {
        // Reset UI widgets that depend on an active node. I disable each
        // widget and then reset them to avoid sending unnecessary signals.
        m_actorTextEdit->setEnabled(false);
        m_actorTextEdit->clear();

        if (!dialogueDataPtr)
        {
            return;
        }

        // Update the node editor widget
        m_actorTextEdit->setEnabled(true);
        m_actorTextEdit->setPlainText(dialogueDataPtr->GetActorText().c_str());
    }

    void ConversationEditorMainWindow::OnActorTextChanged()
    {
    }

    void ConversationEditorMainWindow::OnFileOpenTriggered()
    {
        // For simplicity, I store the filepath in an AZStd::string instead of a QString.
        AZStd::string fullFilePath;
        {
            const QString qFileName =
                QFileDialog::getOpenFileName(nullptr, "Open Dialogue Graph", "", Conversation::ConversationAsset::SOURCE_EXTENSION_PATTERN);
            fullFilePath = qFileName.toStdString().c_str();
        }

        if (fullFilePath.empty())
        {
            return;
        }

        // Get only the name of the file from the full path so we can use it as the name of the tab when we open it.
        AZStd::string fileName = "";
        AZ::StringFunc::Path::GetFileName(fullFilePath.c_str(), fileName);
        AZ_Assert(!fileName.empty(), "Could not retrieve the name of the file.");

        // Create the widget that will be displayed in the editor.
        GraphCanvas::EditorDockWidget* dockWidget = CreateDockWidget(fileName.c_str(), this);
        const GraphCanvas::GraphId graphId = dockWidget->GetGraphId();
        GraphCanvas::SceneRequestBus::Event(graphId, &GraphCanvas::SceneRequests::SetMimeType, m_config->m_mimeType);

        GraphCanvas::AssetEditorCentralDockWindow* centralDockWindow = GetCentralDockWindow();
        centralDockWindow->OnEditorOpened(dockWidget);

        AZStd::shared_ptr<ConversationDocument> dialogueDoc(AZ::Utils::LoadObjectFromFile<ConversationDocument>(fullFilePath.c_str()));
        AZStd::shared_ptr<ConversationGraph> graph = dialogueDoc->m_graphPtr;
        // PostLoadSetup needs to be called to setup the connections and pointers.
        graph->PostLoadSetup(GetGraphContext());

        m_graphs[graphId] = graph;

        GraphModelIntegration::GraphManagerRequestBus::Broadcast(
            &GraphModelIntegration::GraphManagerRequests::CreateGraphController, graphId, graph);
        GraphModelIntegration::GraphControllerNotificationBus::MultiHandler::BusConnect(graphId);
    }

    void ConversationEditorMainWindow::OnFileSaveTriggered()
    {
        // Get the file name from the user.
        const QString fileName =
            QFileDialog::getSaveFileName(nullptr, "Save File", "", Conversation::ConversationAsset::SOURCE_EXTENSION_PATTERN);

        if (fileName.isEmpty())
        {
            return;
        }

        ConversationDocument dialoguedoc;

        GraphCanvas::GraphId id = this->GetActiveGraphCanvasGraphId();
        AZStd::shared_ptr<ConversationGraph> graph = azrtti_cast<ConversationGraph*>(this->GetGraphById(id));
        if (!graph)
        {
            AZ_Printf("ConversationEditorMainWindow", "Cannot cast to DialogueGraph!");
        }

        dialoguedoc.m_graphPtr = graph;
        AZ::Utils::SaveObjectToFile<ConversationDocument>(fileName.toStdString().c_str(), AZ::DataStream::ST_JSON, &dialoguedoc);
    }

    ConversationAssetEditorWindowConfig::ConversationAssetEditorWindowConfig()
        : m_graphContext(new ConversationGraphContext)
    {
    }
} // namespace ConversationEditor

#include <ConversationEditor/moc_ConversationEditorMainWindow.cpp>