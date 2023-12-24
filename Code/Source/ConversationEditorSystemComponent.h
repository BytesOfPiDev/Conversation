
#pragma once

#include "Conversation/ConversationTypeIds.h"
#include "ConversationSystemComponent.h"

#include "Atom/RPI.Reflect/Model/ModelAsset.h"
#include "Atom/RPI.Reflect/System/AnyAsset.h"
#include "AtomToolsFramework/Graph/DynamicNode/DynamicNodeManager.h"
#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Component/Component.h"
#include "AzToolsFramework/API/ToolsApplicationAPI.h"
#include "AzToolsFramework/ActionManager/ActionManagerRegistrationNotificationBus.h"

namespace ConversationEditor
{
    class ConversationEditorSystemComponent
        : public Conversation::ConversationSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
        , private AzToolsFramework::ActionManagerRegistrationNotificationBus::
              Handler
    {
        using BaseSystemComponent = ConversationSystemComponent;

    public:
        AZ_COMPONENT(
            ConversationEditorSystemComponent,
            ConversationEditorSystemComponentTypeId,
            BaseSystemComponent); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationEditorSystemComponent); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        ConversationEditorSystemComponent();
        ~ConversationEditorSystemComponent() override;

    private:
        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void OnActionRegistrationHook() override;
        void OnMenuBindingHook() override;
        void OpenConversationCanvas(AZStd::string const& sourcePath);

    private:
        AtomToolsFramework::DynamicNodeManager m_conversationEditorNodeManager;
        AZ::Data::Asset<ScriptEvents::ScriptEventsAsset> m_editorScriptEvents;
        AZStd::map<AZ::Crc32, AZStd::string> m_functionIdToNameMap;
    };
} // namespace ConversationEditor
