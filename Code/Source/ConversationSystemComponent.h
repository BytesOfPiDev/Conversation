
#pragma once

#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData.h"
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <Conversation/ConversationAsset.h>
#include <Conversation/ConversationBus.h>

namespace Conversation
{
    class ConversationSystemComponent
        : public AZ::Component
        , protected ConversationRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(
            ConversationSystemComponent,
            ConversationSystemComponentTypeId); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationSystemComponent); // NOLINT

        ConversationSystemComponent();
        ~ConversationSystemComponent() override;

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // ConversationRequestBus interface implementation

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
        AZStd::vector<DialogueData> m_dialogues;
    };

} // namespace Conversation
