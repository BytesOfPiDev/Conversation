#pragma once

#if !defined(Q_MOC_RUN)
#include <AtomToolsFramework/Document/AtomToolsDocumentInspector.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentMainWindow.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsInspector.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportToolBar.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportWidget.h>
#include <AtomToolsFramework/Graph/GraphViewSettings.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <GraphCanvas/Styling/StyleManager.h>
#include <GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.h>
#include <GraphCanvas/Widgets/GraphCanvasEditor/GraphCanvasAssetEditorMainWindow.h>
#include <GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.h>
#include <GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.h>
#include <GraphCanvas/Widgets/NodePalette/NodePaletteWidget.h>

#include <QTranslator>
#endif

namespace ConversationEditor
{
    class ConversationCanvasMainWindow : public AtomToolsFramework::AtomToolsDocumentMainWindow
    {
        Q_OBJECT

    public:
        AZ_RTTI( // NOLINT
            ConversationCanvasMainWindow,
            "F9E792DF-2A04-42A7-B277-FCBA722A05F9",
            AtomToolsFramework::AtomToolsDocumentMainWindow);
        AZ_CLASS_ALLOCATOR(ConversationCanvasMainWindow, AZ::SystemAllocator); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationCanvasMainWindow); // NOLINT

        using Base = AtomToolsFramework::AtomToolsDocumentMainWindow;

        ConversationCanvasMainWindow(
            AZ::Crc32 const& toolId, AtomToolsFramework::GraphViewSettingsPtr graphViewSettingsPtr, QWidget* parent = nullptr);
        ~ConversationCanvasMainWindow() override = default;

    protected:
        void ResizeViewportRenderTarget(AZ::u32 width, AZ::u32 height) override;
        void LockViewportRenderTargetSize(AZ::u32 width, AZ::u32 height) override;
        void UnlockViewportRenderTargetSize() override;

        void OnDocumentOpened(AZ::Uuid const& documentId) override;

        void PopulateSettingsInspector(AtomToolsFramework::InspectorWidget* inspector) const override;
        void OnSettingsDialogClosed() override;
        auto GetHelpDialogText() const -> AZStd::string override;

    private:
        AtomToolsFramework::AtomToolsDocumentInspector* m_documentInspector = {};
        AtomToolsFramework::EntityPreviewViewportSettingsInspector* m_viewportSettingsInspector = {};
        AtomToolsFramework::EntityPreviewViewportToolBar* m_toolBar = {};
        AtomToolsFramework::EntityPreviewViewportWidget* m_conversationViewport = {};
        AtomToolsFramework::GraphViewSettingsPtr m_graphViewSettingsPtr = {};
        GraphCanvas::BookmarkDockWidget* m_bookmarkDockWidget = {};
        GraphCanvas::NodePaletteDockWidget* m_nodePalette = {};
        GraphCanvas::StyleManager m_styleManager;
        QTranslator m_translator;
        mutable AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> m_conversationCanvasCompileSettingsGroup;
    };
} // namespace ConversationEditor
