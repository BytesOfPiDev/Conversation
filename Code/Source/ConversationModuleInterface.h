
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <ConversationSystemComponent.h>

#include <Conversation/DialogueComponent.h>
#include <DialogueLibrary.h>

namespace Conversation
{
    class ConversationModuleInterface : public AZ::Module
    {
    public:
        AZ_RTTI(ConversationModuleInterface, "{abf1988a-779a-458a-b8d3-71864f4c66fc}", AZ::Module);
        AZ_CLASS_ALLOCATOR(ConversationModuleInterface, AZ::SystemAllocator, 0);

        ConversationModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(), { ConversationSystemComponent::CreateDescriptor(), DialogueComponent::CreateDescriptor() });

            AZStd::vector<AZ::ComponentDescriptor*> componentDescriptors(DialogueLibrary::GetComponentDescriptors());
            m_descriptors.insert(m_descriptors.end(), componentDescriptors.begin(), componentDescriptors.end());
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<ConversationSystemComponent>(),
            };
        }
    };
} // namespace Conversation
