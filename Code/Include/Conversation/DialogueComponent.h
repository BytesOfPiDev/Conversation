#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/std/parallel/lock.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponentBus.h>

namespace Conversation
{
    /**
     * Represents a conversation.
     *
     * Currently, an entity can only use one instance of this component. However,
     * it would be useful to allow multiple instances in the future in order to allow
     * separation of different conversations that may be added to the entity later.
     *
     * The m_owner AZ::EntityId is the entity you want to "own" the dialogue. This
     * allows you to have the DialogueComponent on a different entity than the one
     * it's attached to - it will listen to requests and send notifications using
     * that AZ::EntityId.
     *
     * @author Andre
     */
    class DialogueComponent
        : public AZ::Component
        , public AZ::EntityBus::Handler
        , public DialogueComponentRequestBus::Handler
        , public ConversationNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(DialogueComponent, "{C7AFDF51-ECCC-4BD3-8A56-0763ED87CB5B}");

        DialogueComponent() = default;
        ~DialogueComponent() override;

        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void AddDialogue(const DialogueData /*dialogueDataToAdd*/, const DialogueId& parentDialogueId = DialogueId::CreateNull()) override;
        
        void ClearData() override
        {
            m_conversationAsset.Reset();
        }

        AZ::Data::Asset<ConversationAsset> GetConversationData() const override
        {
            return m_conversationAsset;
        }

    private:
        AZ::EntityId m_owner;
        AZ::Data::Asset<ConversationAsset> m_conversationAsset;
    };
} // namespace Conversation
