#pragma once

#include <Atom/RHI/FactoryManagerBus.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentApplication.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentNotificationBus.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsSystem.h>
#include <AtomToolsFramework/Graph/DynamicNode/DynamicNodeManager.h>
#include <AtomToolsFramework/Graph/GraphTemplateFileDataCache.h>
#include <AzCore/Component/Component.h>
#include <AzToolsFramework/API/EditorWindowRequestBus.h>
#include <GraphModel/Model/GraphContext.h>
#include <Tools/Window/ConversationCanvasMainWindow.h>

namespace ConversationEditor
{
    class ConversationCanvasApplication
        : public AtomToolsFramework::AtomToolsDocumentApplication
        , private AzToolsFramework::EditorWindowRequestBus::Handler
        , private AZ::RHI::FactoryManagerNotificationBus::Handler
        , private AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(ConversationCanvasApplication, AZ::SystemAllocator); // NOLINT
        AZ_RTTI( // NOLINT
            ConversationCanvasApplication,
            "8417F15F-B290-47A4-9996-1954F3D153EB",
            AtomToolsFramework::AtomToolsDocumentApplication); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationCanvasApplication); // NOLINT

        using Base = AtomToolsFramework::AtomToolsDocumentApplication;

        ConversationCanvasApplication(int* argc, char*** argv);
        ~ConversationCanvasApplication() override;

        // AzFramework::Application overrides...
        void Reflect(AZ::ReflectContext* context) override;
        auto GetCurrentConfigurationName() const -> const char* override;
        void StartCommon(AZ::Entity* systemEntity) override;
        void Destroy() override;

    private:
        // AtomToolsFramework::AtomToolsApplication overrides...
        auto GetCriticalAssetFilters() const -> AZStd::vector<AZStd::string> override;

        // AzToolsFramework::EditorWindowRequests::Bus::Handler
        auto GetAppMainWindow() -> QWidget* override;

        // AZ::RHI::FactoryManagerNotificationBus::Handler overrides...
        void FactoryRegistered() override;

        void InitDynamicNodeManager();
        void InitDynamicNodeEditData();
        void InitSharedGraphContext();
        void InitGraphViewSettings();
        void InitConversationGraphDocumentType();
        void InitConversationGraphNodeDocumentType();
        void InitDialogueSourceDataDocumentType();
        void InitMainWindow();
        void InitDefaultDocument();

    private:
        AZStd::unique_ptr<ConversationCanvasMainWindow> m_window;
        AZStd::unique_ptr<AtomToolsFramework::EntityPreviewViewportSettingsSystem> m_viewportSettingsSystem;
        AZStd::unique_ptr<AtomToolsFramework::DynamicNodeManager> m_dynamicNodeManager;
        AZStd::shared_ptr<GraphModel::GraphContext> m_graphContext;
        AZStd::shared_ptr<AtomToolsFramework::GraphTemplateFileDataCache> m_graphTemplateFileDataCache;
        AtomToolsFramework::GraphViewSettingsPtr m_graphViewSettingsPtr;
    };

} // namespace ConversationEditor
