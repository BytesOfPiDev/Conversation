/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root
 * of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include "ConversationCanvasTypeIds.h"

#include "Window/ConversationCanvasMainWindow.h"
#include <Atom/RHI/FactoryManagerBus.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentApplication.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportSettingsSystem.h>
#include <AtomToolsFramework/Graph/AssetStatusReporterSystem.h>
#include <AtomToolsFramework/Graph/DynamicNode/DynamicNodeManager.h>
#include <AtomToolsFramework/Graph/GraphTemplateFileDataCache.h>
#include <AzToolsFramework/API/EditorWindowRequestBus.h>
#include <GraphModel/Model/GraphContext.h>

namespace ConversationCanvas
{
    //! The main application class for Conversation Canvas, setting up top level
    //! systems, document types, and the main window.
    class ConversationCanvasApplication
        : public AtomToolsFramework::AtomToolsDocumentApplication
        , private AzToolsFramework::EditorWindowRequestBus::Handler
        , private AZ::RHI::FactoryManagerNotificationBus::Handler
        , private AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(ConversationCanvasApplication, AZ::SystemAllocator)
        AZ_TYPE_INFO(
            ConversationCanvas::ConversationCanvasApplication,
            ConversationCanvasApplicationTypeId);

        using Base = AtomToolsFramework::AtomToolsDocumentApplication;

        ConversationCanvasApplication(int* argc, char*** argv);
        ~ConversationCanvasApplication();

        // AzFramework::Application overrides...
        void Reflect(AZ::ReflectContext* context) override;
        char const* GetCurrentConfigurationName() const override;
        void StartCommon(AZ::Entity* systemEntity) override;
        void Destroy() override;

    private:
        // AtomToolsFramework::AtomToolsApplication overrides...
        AZStd::vector<AZStd::string> GetCriticalAssetFilters() const override;

        // AzToolsFramework::EditorWindowRequests::Bus::Handler
        QWidget* GetAppMainWindow() override;

        // AZ::RHI::FactoryManagerNotificationBus::Handler overrides...
        void FactoryRegistered() override;

        void InitDynamicNodeManager();
        void InitDynamicNodeEditData();
        void InitSharedGraphContext();
        void InitGraphViewSettings();
        void InitConversationGraphDocumentType();
        void InitConversationGraphNodeDocumentType();
        void InitMainWindow();
        void InitDefaultDocument();

        AZStd::unique_ptr<ConversationCanvasMainWindow> m_window;
        AZStd::unique_ptr<AtomToolsFramework::DynamicNodeManager>
            m_dynamicNodeManager;
        AZStd::unique_ptr<AtomToolsFramework::AssetStatusReporterSystem>
            m_assetStatusReporterSystem;
        AZStd::shared_ptr<GraphModel::GraphContext> m_graphContext;
        AZStd::shared_ptr<AtomToolsFramework::GraphTemplateFileDataCache>
            m_graphTemplateFileDataCache;
        AtomToolsFramework::GraphViewSettingsPtr m_graphViewSettingsPtr;
    };
} // namespace ConversationCanvas
