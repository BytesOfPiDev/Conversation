
#pragma once

#include <ConversationSystemComponent.h>

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

namespace Conversation
{
    /// System component for Conversation editor
    class ConversationEditorSystemComponent
        : public ConversationSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = ConversationSystemComponent;
    public:
        AZ_COMPONENT(ConversationEditorSystemComponent, "{bcd6e4b7-5632-46b4-ba76-b3c4e0b98310}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        ConversationEditorSystemComponent();
        ~ConversationEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // AzToolsFramework::EditorEventsBus overrides ...
        void NotifyRegisterViews() override;
    };
} // namespace Conversation
