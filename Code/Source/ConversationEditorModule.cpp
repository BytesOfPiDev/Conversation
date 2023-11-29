
#include "Builder/ConversationAssetBuilderComponent.h"
#include "Components/EditorDialogueComponent.h"
#include "ConversationEditorSystemComponent.h"
#include "ConversationModuleInterface.h"


namespace ConversationEditor
{
    class ConversationEditorModule : public Conversation::ConversationModuleInterface
    {
    public:
        AZ_RTTI(ConversationEditorModule, "{0891f520-4159-453a-9663-8bdb2931b125}", ConversationModuleInterface); // NOLINT
        AZ_CLASS_ALLOCATOR(ConversationEditorModule, AZ::SystemAllocator, 0); // NOLINT

        ConversationEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                { ConversationEditorSystemComponent::CreateDescriptor(), DialogueAssetBuilderComponent::CreateDescriptor(),
                  EditorDialogueComponent::CreateDescriptor() });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        [[nodiscard]] auto GetRequiredSystemComponents() const -> AZ::ComponentTypeList override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<ConversationEditorSystemComponent>(),
            };
        }
    };
} // namespace ConversationEditor

AZ_DECLARE_MODULE_CLASS(Gem_Conversation, ConversationEditor::ConversationEditorModule) // NOLINT
