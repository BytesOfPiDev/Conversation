
#include <ConversationModuleInterface.h>
#include <ConversationEditorSystemComponent.h>

void InitConversationResources()
{
    // We must register our Qt resources (.qrc file) since this is being loaded from a separate module (gem)
    Q_INIT_RESOURCE(Conversation);
}

namespace Conversation
{
    class ConversationEditorModule
        : public ConversationModuleInterface
    {
    public:
        AZ_RTTI(ConversationEditorModule, "{0891f520-4159-453a-9663-8bdb2931b125}", ConversationModuleInterface);
        AZ_CLASS_ALLOCATOR(ConversationEditorModule, AZ::SystemAllocator, 0);

        ConversationEditorModule()
        {
            InitConversationResources();

            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                ConversationEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<ConversationEditorSystemComponent>(),
            };
        }
    };
}// namespace Conversation

AZ_DECLARE_MODULE_CLASS(Gem_Conversation, Conversation::ConversationEditorModule)
