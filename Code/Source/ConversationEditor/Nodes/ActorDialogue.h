#pragma once

#include <GraphModel/Model/Node.h>

#include <AzCore/std/smart_ptr/shared_ptr.h>

namespace Conversation
{
	class DialogueData;
}

namespace ConversationEditor
{
	namespace Nodes
	{
		class ActorDialogue
			: public GraphModel::Node
		{
		public:
			AZ_CLASS_ALLOCATOR(ActorDialogue, AZ::SystemAllocator, 0);
            AZ_RTTI(ActorDialogue, "{F42247E3-4896-4BAC-A7B2-5C8892079C6E}", Node);

			static void Reflect(AZ::ReflectContext* context);

			ActorDialogue() = default;
			ActorDialogue(GraphModel::GraphPtr graph);
			~ActorDialogue() override = default;

			const char* GetSubTitle() const override { return "An actor's dialogue."; }
			const char* GetTitle() const override { return "Actor Dialogue"; }

		protected:
			bool CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition) const override;
			void RegisterSlots() override;
		};
	}
}
