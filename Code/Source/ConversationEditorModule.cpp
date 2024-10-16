
#include "Builder/ConversationAssetBuilderComponent.h"
#include "ConversationEditorSystemComponent.h"
#include "ConversationModuleInterface.h"
#include "Tools/Components/EditorDialogueComponent.h"

namespace ConversationEditor
{
    class ConversationEditorModule
        : public Conversation::ConversationModuleInterface
    {
    public:
        AZ_RTTI(
            ConversationEditorModule,
            "{0891F520-4159-453A-9663-8BDB2931B125}",
            ConversationModuleInterface); // NOLINT
        AZ_CLASS_ALLOCATOR(
            ConversationEditorModule, AZ::SystemAllocator, 0); // NOLINT

        ConversationEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into
            // m_descriptors here. Add ALL components descriptors associated
            // with this gem to m_descriptors. This will associate the
            // AzTypeInfo information for the components with the the
            // SerializeContext, BehaviorContext and EditContext. This happens
            // through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                {
                    EditorDialogueComponent::CreateDescriptor(),
                    ConversationEditorSystemComponent::CreateDescriptor(),
                    DialogueAssetBuilderComponent::CreateDescriptor(),
                });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        [[nodiscard]] auto GetRequiredSystemComponents() const
            -> AZ::ComponentTypeList override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<ConversationEditorSystemComponent>(),
            };
        }
    };
} // namespace ConversationEditor

AZ_DECLARE_MODULE_CLASS(
    Gem_Conversation, ConversationEditor::ConversationEditorModule) // NOLINT
