
#pragma once

#include <ConversationSystemComponent.h>

#include <AtomToolsFramework/Graph/DynamicNode/DynamicNodeManager.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <ScriptEvents/ScriptEvent.h>

namespace ConversationEditor
{
    class ConversationEditorSystemComponent
        : public Conversation::ConversationSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = ConversationSystemComponent;

    public:
        AZ_COMPONENT(ConversationEditorSystemComponent, "{bcd6e4b7-5632-46b4-ba76-b3c4e0b98310}", BaseSystemComponent); // NOLINT
        static void Reflect(AZ::ReflectContext* context);

        ConversationEditorSystemComponent();
        ~ConversationEditorSystemComponent() override;

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Init() override;
        void Activate() override;
        void Deactivate() override;

    private:
        AtomToolsFramework::DynamicNodeManager m_conversationEditorNodeManager;
        AZ::Data::Asset<ScriptEvents::ScriptEventsAsset> m_editorScriptEvents;
        AZStd::map<AZ::Crc32, AZStd::string> m_functionIdToNameMap;
    };
} // namespace ConversationEditor
