
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <Conversation/ConversationBus.h>

namespace Conversation
{
    enum class ConversationStatus
    {
        Inactive,
        Starting,
        Active,
        Aborting,
        Ending
    };

    struct ActiveConversationData
    {
        AZ::EntityId OwningEntity;
        DialogueData CurrentlyActiveDialogue;
        AZStd::vector<DialogueData> AvailableResponses;
    };

    class ConversationSystemComponent
        : public AZ::Component
        , protected ConversationRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(ConversationSystemComponent, "{30f94275-e830-466f-b1c6-140156911232}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        ConversationSystemComponent();
        ~ConversationSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // ConversationRequestBus interface implementation

        void AbortConversation() override;
        void EndConversation() override;
        void StartConversation(const AZ::EntityId entityId) override;
        void SelectResponseById(const DialogueId&) override;

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        ////////////////////////////////////////////////////////////////////////

    private:
        AZStd::unique_ptr<ConversationAssetHandler> m_conversationAssetHandler;
        ConversationStatus m_currentConversationStatus;
        AZ::Data::Asset<ConversationAsset> m_currentConversationAsset;
        AZStd::unique_ptr<ActiveConversationData> m_currentConversationData;
    };

} // namespace Conversation
