#pragma once

#if !defined(Q_MOC_RUN) // Required, or else AUTOMOC uses AZ namespace.
#include <GraphModel/Integration/EditorMainWindow.h>

#include <Conversation/DialogueData.h>
#include <ConversationEditor/Common.h>
#include <ConversationEditor/ConversationGraphContext.h>
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

namespace ConversationEditor
{
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

        QAction* AddFileOpenAction(QMenu* menu) override;
        QAction* AddFileSaveAction(QMenu* menu) override;

        void OnSelectionChanged() override;

    signals:
        void nodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr);

    private:
        void OnNodeSelectionChanged(const Conversation::DialogueDataPtr dialogueDataPtr);
        void OnActorTextChanged();

    public:
        void OnFileOpenTriggered();
        void OnFileSaveTriggered();

        private:
        Conversation::DialogueDataPtr GetActiveNodeDialoguePointer() const;
        

    private:
        ConversationAssetEditorWindowConfig* m_config;
        GraphModel::GraphContextPtr m_graphContext;
        GraphCanvas::GraphId m_lastActiveGraphId;

        Conversation::DialogueDataPtr m_activeDialogueDataPointer;

        AzQtComponents::StyledDockWidget* m_rightDockWidget;
        AzQtComponents::StyledDockWidget* m_bottomDockWidget;

        AZStd::shared_ptr<QAction> m_fileOpenAction;
        AZStd::shared_ptr<QAction> m_fileSaveAction;

        QPlainTextEdit* m_actorTextEdit;
    };
} // namespace ConversationEditor
