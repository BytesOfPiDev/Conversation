#pragma once

#include "AzCore/Script/ScriptAsset.h"
#include "AzFramework/Entity/EntityDebugDisplayBus.h"
#include "AzToolsFramework/ToolsComponents/EditorComponentBase.h"

#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationTypeIds.h"

namespace ConversationEditor
{
    class EditorDialogueComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , private AzFramework::EntityDebugDisplayEventBus::Handler
    {
    public:
        AZ_COMPONENT( // NOLINT
            EditorDialogueComponent,
            EditorDialogueComponentTypeId,
            AzToolsFramework::Components::EditorComponentBase);

        AZ_DISABLE_COPY_MOVE( // NOLINT
            EditorDialogueComponent);

        static void Reflect(AZ::ReflectContext* context);

        EditorDialogueComponent() = default;
        ~EditorDialogueComponent() override = default;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void DisplayEntityViewport(
            AzFramework::ViewportInfo const& viewportInfo,
            AzFramework::DebugDisplayRequests& debugDisplay) override;

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);

    protected:
        // Turn an editor component into a runtime component.
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        Conversation::DialogueComponentConfig m_config{};
    };
} // namespace ConversationEditor
