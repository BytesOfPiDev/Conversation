#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Module/Module.h"

#include "Components/ConversationAssetRefComponent.h"
#include "ConversationSystemComponent.h"
#include "DialogueComponent.h"

namespace Conversation
{
    class ConversationModuleInterface : public AZ::Module
    {
    public:
        AZ_RTTI(
            ConversationModuleInterface,
            "{ABF1988A-779A-458A-B8D3-71864F4C66FC}",
            AZ::Module);
        AZ_CLASS_ALLOCATOR(ConversationModuleInterface, AZ::SystemAllocator, 0);

        ConversationModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into
            // m_descriptors here. Add ALL components descriptors associated
            // with this gem to m_descriptors. This will associate the
            // AzTypeInfo information for the components with the the
            // SerializeContext, BehaviorContext and EditContext. This happens
            // through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                { ConversationSystemComponent::CreateDescriptor(),
                  DialogueComponent::CreateDescriptor(),
                  ConversationAssetRefComponent::CreateDescriptor() });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        [[nodiscard]] auto GetRequiredSystemComponents() const
            -> AZ::ComponentTypeList override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<ConversationSystemComponent>(),
            };
        }
    };
} // namespace Conversation
