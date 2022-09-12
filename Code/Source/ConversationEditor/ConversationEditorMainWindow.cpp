#include <ConversationEditor/ConversationEditorMainWindow.h>

#include <AtomToolsFramework/DynamicProperty/DynamicPropertyGroup.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/UserSettings/UserSettingsProvider.h>
#include <AzQtComponents/Components/StyledDockWidget.h>
#include <Conversation/DialogueData.h>
#include <ConversationEditor/Common.h>
#include <ConversationEditor/ConversationDocument.h>
#include <ConversationEditor/DataTypes.h>
#include <ConversationEditor/Nodes/ActorDialogue.h>
#include <ConversationEditor/Nodes/Link.h>
#include <ConversationEditor/Nodes/RootNode.h>
#include <ConversationEditor/SettingsDialog.h>
#include <ConversationEditor/VariableNodePaletteTreeItemTypes.h>
#include <ConversationEditor/ui_ConversationEditorWidget.h>
#include <GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasEditorDockWidget.h>
#include <GraphModel/GraphModelBus.h>
#include <GraphModel/Integration/NodePalette/GraphCanvasNodePaletteItems.h>
#include <GraphModel/Integration/NodePalette/InputOutputNodePaletteItem.h>
#include <GraphModel/Integration/NodePalette/StandardNodePaletteItem.h>
#include <QFileDialog>
#include <QMenuBar>

#include <AzQtComponents/Components/Widgets/Card.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>

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
        m_userSettings = AZ::UserSettings::CreateFind<ConversationEditorSettings>(ConversationEditorSettingsId, AZ::UserSettings::CT_LOCAL);

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
        AZ::UserSettingsOwnerRequestBus::Event(AZ::UserSettings::CT_LOCAL, &AZ::UserSettingsOwnerRequestBus::Events::SaveSettings);
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

        m_conversationEditorUi = AZStd::make_unique<Ui::ConversationEditorWidget>();
        QWidget* conversationEditor = new QWidget;
        m_conversationEditorUi->setupUi(conversationEditor);
        m_bottomDockWidget->setWidget(conversationEditor);

        m_actorTextEdit = m_conversationEditorUi->dialogueEdit;
        m_conversationEditorUi->speakerTagComboBox->addItem("owner");
        m_conversationEditorUi->speakerTagComboBox->addItem("player");

        m_propertyEditorCard = new AzQtComponents::Card(m_rightDockWidget);
        m_propertyEditor = aznew AzToolsFramework::ReflectedPropertyEditor(m_propertyEditorCard);
        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
        m_propertyEditor->Setup(serializeContext, nullptr, false);
        m_propertyEditorCard->setContentWidget(m_propertyEditor);
        m_rightDockWidget->setWidget(m_propertyEditorCard);

        m_settingsDialog = new ConversationSettingsDialog(m_userSettings.get(), this);

        QObject::connect(
            this, &ConversationEditorMainWindow::nodeSelectionChanged, this, &ConversationEditorMainWindow::OnNodeSelectionChanged);
        QObject::connect(m_actorTextEdit, &QPlainTextEdit::textChanged, this, &ConversationEditorMainWindow::OnActorTextChanged);
        QObject::connect(
            m_conversationEditorUi->speakerTagComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSpeakerTagChanged(int)));
    }

    QMenu* ConversationEditorMainWindow::AddFileMenu()
    {
        return GraphModelIntegration::EditorMainWindow::AddFileMenu();
    }

    QMenu* ConversationEditorMainWindow::AddEditMenu()
    {
        QMenu* editMenu = GraphModelIntegration::EditorMainWindow::AddEditMenu();

        m_editSettingsAction = new QAction(tr("&Settings"), editMenu);

        editMenu->addSeparator();
        editMenu->addAction(m_editSettingsAction);

        QObject::connect(m_editSettingsAction, &QAction::triggered, this, &ConversationEditorMainWindow::OnEditSettingsTriggered);

        return editMenu;
    }

    QAction* ConversationEditorMainWindow::AddFileOpenAction(QMenu* menu)
    {
        m_fileOpenAction = EditorMainWindow::AddFileOpenAction(menu);

        QObject::connect(
            m_fileOpenAction, &QAction::triggered,
            [this]
            {
                OnFileOpenTriggered();
            });

        menu->addAction(m_fileOpenAction);

        return m_fileOpenAction;
    }

    QAction* ConversationEditorMainWindow::AddFileSaveAction(QMenu* menu)
    {
        m_fileSaveAction = EditorMainWindow::AddFileSaveAction(menu);

        QObject::connect(
            m_fileSaveAction, &QAction::triggered,
            [this]
            {
                OnFileSaveTriggered();
            });

        menu->addAction(m_fileSaveAction);

        return m_fileSaveAction;
    }

    void ConversationEditorMainWindow::OnSelectionChanged()
    {
        // The active dialogue should be set to null so programatic changes don't automatically try to
        // put their new contents (usually just default/empty values due to the node changing) into
        // the dialogue node that's about to be active. OnNodeSelectionChanged will be responsible for
        // setting it, which should be trigged after the emit call later in this function.
        m_activeDialogueDataPointer = nullptr;

        GraphCanvas::AssetEditorMainWindow::OnSelectionChanged();
        emit nodeSelectionChanged(GetActiveNodeDialoguePointer());
    }

    void ConversationEditorMainWindow::OnNodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr)
    {
        m_propertyEditor->ClearInstances();
        if (!dialogueDataPtr)
        {
            m_actorTextEdit->setEnabled(false);
            m_actorTextEdit->clear();

            m_conversationEditorUi->speakerTagComboBox->setEnabled(false);

            return;
        }

        m_actorTextEdit->setEnabled(true);
        m_actorTextEdit->setPlainText(dialogueDataPtr->GetActorText().c_str());

        m_conversationEditorUi->speakerTagComboBox->setEnabled(true);
        const char* const speakerTag = dialogueDataPtr->GetSpeaker().c_str();

        const int speakerTagComboBoxIndex = m_conversationEditorUi->speakerTagComboBox->findText(speakerTag, Qt::MatchFixedString);
        if (speakerTagComboBoxIndex >= 0)
        {
            m_conversationEditorUi->speakerTagComboBox->setCurrentIndex(speakerTagComboBoxIndex);
        }

        m_conversationEditorUi->nodeTabWidget->setEnabled(true);
        m_conversationEditorUi->scriptsTab->setEnabled(true);
        m_conversationEditorUi->scriptListWidget->setEnabled(true);
        m_propertyEditor->AddInstance(dialogueDataPtr.get());
        if (auto graph = azrtti_cast<ConversationGraph*>(this->GetGraphById(this->GetActiveGraphCanvasGraphId())))
        {
            m_propertyEditor->AddInstance(m_userSettings.get());
        }

        m_propertyEditor->InvalidateAll();

        // With the active dialogue being set, signals and slots are now safe to use this cache'd pointer
        // to make changes to the active node.
        m_activeDialogueDataPointer = dialogueDataPtr;
    }

    void ConversationEditorMainWindow::OnActorTextChanged()
    {
        const Conversation::DialogueDataPtr dialogueDataPtr = GetActiveNodeDialoguePointer();
        if (dialogueDataPtr)
        {
            dialogueDataPtr->SetActorText(m_actorTextEdit->toPlainText().toStdString().c_str());
        }
    }

    void ConversationEditorMainWindow::OnEditSettingsTriggered()
    {
        m_settingsDialog->setModal(true);
        m_settingsDialog->show();
    }

    void ConversationEditorMainWindow::OnSpeakerTagChanged([[maybe_unused]] int index)
    {
        if (!m_activeDialogueDataPointer)
        {
            return;
        }

        m_activeDialogueDataPointer->SetSpeaker(m_conversationEditorUi->speakerTagComboBox->itemText(index).toStdString().c_str());
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

    Conversation::DialogueDataPtr ConversationEditorMainWindow::GetActiveNodeDialoguePointer() const
    {
        // Find the node that is currently selected, if any.
        GraphModel::NodePtrList result;
        GraphModelIntegration::GraphControllerRequestBus::EventResult(
            result, this->GetActiveGraphCanvasGraphId(), &GraphModelIntegration::GraphControllerRequestBus::Events::GetSelectedNodes);

        // We're only interested in the first node right now, but it is possible the user has multiple nodes selected.
        // \todo At some point, we need to handle multiple selection scenarios to make sure the right nodes are read
        // and edited.
        const GraphModel::SlotPtr dialogueDataSlotPtr = result.empty() ? nullptr : result.front()->GetSlot(CommonSlotNames::DIALOGUEDATA);
        return dialogueDataSlotPtr ? dialogueDataSlotPtr->GetValue<Conversation::DialogueDataPtr>() : nullptr;
    }

    ConversationAssetEditorWindowConfig::ConversationAssetEditorWindowConfig()
        : m_graphContext(new ConversationGraphContext)
    {
    }
} // namespace ConversationEditor

#include <ConversationEditor/moc_ConversationEditorMainWindow.cpp>