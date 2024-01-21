#pragma once

#if !defined(Q_MOC_RUN)
#include "AtomToolsFramework/Document/AtomToolsDocumentInspector.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentMainWindow.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsInspector.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportToolBar.h"
#include "AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportWidget.h"
#include "AtomToolsFramework/Graph/GraphViewSettings.h"
#include "GraphCanvas/Components/SceneBus.h"
#include "GraphCanvas/Styling/StyleManager.h"
#include "GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.h"
#include "GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.h"

#include <QTranslator>
#endif

namespace ConversationEditor
{
    class ConversationCanvasMainWindow
        : public AtomToolsFramework::AtomToolsDocumentMainWindow
        , protected GraphCanvas::SceneNotificationBus::Handler
    {
        Q_OBJECT;

    public:
        AZ_RTTI( // NOLINT(modernize-use-trailing-return-type)
            ConversationCanvasMainWindow,
            "{F9E792DF-2A04-42A7-B277-FCBA722A05F9}",
            AtomToolsFramework::AtomToolsDocumentMainWindow);

        AZ_CLASS_ALLOCATOR( // NOLINT(*-pro-type-reinterpret-cast,
                            // *-pro-bounds-array-to-pointer-decay)
            ConversationCanvasMainWindow,
            AZ::SystemAllocator);

        AZ_DISABLE_COPY_MOVE( // NOLINT(modernize-use-trailing-return-type)
            ConversationCanvasMainWindow);

        using Base = AtomToolsFramework::AtomToolsDocumentMainWindow;

        ConversationCanvasMainWindow(
            AZ::Crc32 const& toolId,
            AtomToolsFramework::GraphViewSettingsPtr const&
                graphViewSettingsPtr,
            QWidget* parent = nullptr);

        ~ConversationCanvasMainWindow() override = default;

    protected:
        void ResizeViewportRenderTarget(AZ::u32 width, AZ::u32 height) override;
        void LockViewportRenderTargetSize(
            AZ::u32 width, AZ::u32 height) override;
        void UnlockViewportRenderTargetSize() override;

        void OnDocumentOpened(AZ::Uuid const& documentId) override;

        void PopulateSettingsInspector(
            AtomToolsFramework::InspectorWidget* inspector) const override;
        void OnSettingsDialogClosed() override;
        auto GetHelpDialogText() const -> AZStd::string override;

        void OnPreNodeDeleted(AZ::EntityId const& nodeId) override;

    private:
        AtomToolsFramework::AtomToolsDocumentInspector*
            m_documentInspector = {};
        AtomToolsFramework::EntityPreviewViewportSettingsInspector*
            m_viewportSettingsInspector = {};
        AtomToolsFramework::EntityPreviewViewportToolBar* m_toolBar = {};
        AtomToolsFramework::EntityPreviewViewportWidget*
            m_conversationViewport = {};
        AtomToolsFramework::GraphViewSettingsPtr m_graphViewSettingsPtr = {};
        GraphCanvas::BookmarkDockWidget* m_bookmarkDockWidget = {};
        GraphCanvas::NodePaletteDockWidget* m_nodePalette = {};
        GraphCanvas::StyleManager m_styleManager;
        QTranslator m_translator;
        mutable AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup>
            m_conversationCanvasCompileSettingsGroup;
    };
} // namespace ConversationEditor
