#pragma once

#include <AzCore/Component/Component.h>
#include <Builder/ConversationAssetBuilderWorker.h>

namespace ConversationEditor
{
    class DialogueAssetBuilderComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(DialogueAssetBuilderComponent, "{197D2409-961A-40ED-84DE-E9DD55B12B96}"); // NOLINT
        AZ_DISABLE_COPY_MOVE(DialogueAssetBuilderComponent); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        DialogueAssetBuilderComponent();
        ~DialogueAssetBuilderComponent() override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    private:
        ConversationAssetBuilderWorker m_dialogueAssetBuilder;
    };
} // namespace ConversationEditor
