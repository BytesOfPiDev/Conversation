#pragma once

#include "Tools/ConversationCanvasTypeIds.h"
#if !defined(Q_MOC_RUN)
#include "AtomToolsFramework/Document/AtomToolsDocumentInspector.h"
#include "AtomToolsFramework/Document/AtomToolsDocumentMainWindow.h"
#include "AtomToolsFramework/Graph/GraphViewSettings.h"
#include "GraphCanvas/Components/SceneBus.h"
#include "GraphCanvas/Styling/StyleManager.h"
#include "GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.h"
#include "GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.h"

#include <QTranslator>
#endif

namespace ConversationCanvas
{
    class ConversationCanvasMainWindow
        : public AtomToolsFramework::AtomToolsDocumentMainWindow
        , protected GraphCanvas::SceneNotificationBus::Handler
    {
        Q_OBJECT;

    public:
        AZ_RTTI(
            ConversationCanvasMainWindow,
            ConversationCanvasMainWindowTypeId,
            AtomToolsFramework::AtomToolsDocumentMainWindow);

        AZ_CLASS_ALLOCATOR(ConversationCanvasMainWindow, AZ::SystemAllocator);

        AZ_DISABLE_COPY_MOVE(ConversationCanvasMainWindow);

        using Base = AtomToolsFramework::AtomToolsDocumentMainWindow;

        ConversationCanvasMainWindow(
            AZ::Crc32 const& toolId,
            AtomToolsFramework::GraphViewSettingsPtr const&
                graphViewSettingsPtr,
            QWidget* parent = nullptr);

        ~ConversationCanvasMainWindow() override = default;

    protected:
        void OnDocumentOpened(AZ::Uuid const& documentId) override;

        void PopulateSettingsInspector(
            AtomToolsFramework::InspectorWidget* inspector) const override;
        void OnSettingsDialogClosed() override;
        auto GetHelpDialogText() const -> AZStd::string;

        void OnPreNodeDeleted(AZ::EntityId const& nodeId) override;

    private:
        AtomToolsFramework::AtomToolsDocumentInspector*
            m_documentInspector = {};
        AtomToolsFramework::GraphViewSettingsPtr m_graphViewSettingsPtr = {};
        GraphCanvas::BookmarkDockWidget* m_bookmarkDockWidget = {};
        GraphCanvas::NodePaletteDockWidget* m_nodePalette = {};
        GraphCanvas::StyleManager m_styleManager;
        QTranslator m_translator;
        mutable AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup>
            m_conversationCanvasCompileSettingsGroup;
    };
} // namespace ConversationCanvas
