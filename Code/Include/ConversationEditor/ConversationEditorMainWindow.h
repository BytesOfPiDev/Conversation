#pragma once

#if !defined(Q_MOC_RUN) // Required, or else AUTOMOC uses AZ namespace.
#include <GraphModel/Integration/EditorMainWindow.h>

#include <Conversation/DialogueData.h>
#include <ConversationEditor/Common.h>
#include <ConversationEditor/ConversationGraphContext.h>
#include <ScriptEvents/ScriptEventsBus.h>
#endif

class QPlainTextEdit;


namespace AzQtComponents
{
    class StyledDockWidget;
    class Card;
} // namespace AzQtComponents

namespace AzToolsFramework
{
    class ReflectedPropertyEditor;
}

namespace Ui
{
    class ConversationEditorWidget;
}

namespace ConversationEditor
{
    class ConversationEditorSettings;
    class ConversationSettingsDialog;

    class ConversationAssetEditorWindowConfig : public GraphCanvas::AssetEditorWindowConfig
    {
    public:
        ConversationAssetEditorWindowConfig();

        [[nodiscard]] GraphCanvas::GraphCanvasTreeItem* CreateNodePaletteRoot() override;
        [[nodiscard]] GraphModel::GraphContextPtr GetGraphContext() const
        {
            return m_graphContext;
        }

    private:
        GraphModel::GraphContextPtr m_graphContext;
    };

    class ConversationEditorMainWindow : public GraphModelIntegration::EditorMainWindow
    {
        Q_OBJECT;

    public:
        ConversationEditorMainWindow(ConversationAssetEditorWindowConfig* config, QWidget* parent = nullptr);
        ~ConversationEditorMainWindow() override;

        void SetupUI();

    protected:
        GraphModel::GraphContextPtr GetGraphContext() const override;

        void OnEditorClosing(GraphCanvas::EditorDockWidget* dockWidget) override;
        void OnEditorOpened(GraphCanvas::EditorDockWidget* dockWidget) override;
        void OnWrapperNodeActionWidgetClicked(
            const AZ::EntityId& wrapperNode,
            const QRect& actionWidgetBoundingRect,
            const QPointF& scenePoint,
            const QPoint& screenPoint) override;

        QMenu* AddFileMenu() override;
        QMenu* AddEditMenu() override;
        QAction* AddFileOpenAction(QMenu* menu) override;
        QAction* AddFileSaveAction(QMenu* menu) override;

        void OnSelectionChanged() override;

    signals:
        void nodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr);

    private slots:
        // Updates the active node's tag with the new
        void OnSpeakerTagChanged(int index);
        void OnNodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr);
        void OnActorTextChanged();
        void OnEditSettingsTriggered();

    private:
        void OnFileOpenTriggered();
        void OnFileSaveTriggered();

        Conversation::DialogueDataPtr GetActiveNodeDialoguePointer() const;

    private:
        ConversationAssetEditorWindowConfig* m_config;
        GraphModel::GraphContextPtr m_graphContext;
        GraphCanvas::GraphId m_lastActiveGraphId;

        Conversation::DialogueDataPtr m_activeDialogueDataPointer;

        AzQtComponents::StyledDockWidget* m_rightDockWidget;
        AzQtComponents::StyledDockWidget* m_bottomDockWidget;
        AzQtComponents::Card* m_propertyEditorCard;
        AzToolsFramework::ReflectedPropertyEditor* m_propertyEditor;

        QAction* m_fileOpenAction = nullptr;
        QAction* m_fileSaveAction = nullptr;
        QAction* m_editSettingsAction = nullptr;
        AZStd::unique_ptr<Ui::ConversationEditorWidget> m_conversationEditorUi;

        QPlainTextEdit* m_actorTextEdit;
        
        AZStd::intrusive_ptr<ConversationEditorSettings> m_userSettings = nullptr;
        QAction* m_settingsAction = nullptr;

        ConversationSettingsDialog* m_settingsDialog;
    };
} // namespace ConversationEditor
