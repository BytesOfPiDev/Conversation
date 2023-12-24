
#include <ConversationModuleInterface.h>
#include <ConversationSystemComponent.h>

namespace Conversation
{
    class ConversationModule : public ConversationModuleInterface
    {
    public:
        AZ_RTTI(
            ConversationModule,
            "{0891f520-4159-453a-9663-8bdb2931b125}",
            ConversationModuleInterface);
        AZ_CLASS_ALLOCATOR(ConversationModule, AZ::SystemAllocator, 0);
    };
} // namespace Conversation

AZ_DECLARE_MODULE_CLASS(Gem_Conversation, Conversation::ConversationModule)
