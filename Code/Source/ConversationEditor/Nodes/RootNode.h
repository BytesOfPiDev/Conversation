#pragma once

#include <GraphModel/Model/Node.h>

namespace ConversationEditor
{
	namespace Nodes
	{
		/**
		 * The first node of a dialogue tree.
		 * 
		 * This node holds the list of greetings that an NPC can use at the start of a conversation.
		 * 
		 * @todo Implement restricting only one of these per graph. Possibly by having it present be default.
		 * @todo Add additional information that describes the dialogue tree as a whole, such as comments.
		 */
		class RootNode
			: public GraphModel::Node
		{
		public:
			AZ_CLASS_ALLOCATOR(RootNode, AZ::SystemAllocator, 0);
			AZ_RTTI(RootNode, "{B3917850-DF15-40A8-A7EB-B6BDB7F3010D}", Node);
	
			static void Reflect(AZ::ReflectContext* context);
			
			RootNode(); /** Required for serialization */
			/**
			 * Creates a RootNode that is added to the graph that is passed in.
			 */
			RootNode(GraphModel::GraphPtr graph);
			~RootNode() override;
	
			const char* GetSubTitle() const override { return "Dialogue Root Subtitle"; }
			const char* GetTitle() const override { return "Dialogue Root"; }
	
		protected:
			void RegisterSlots() override;
	
		};
	}
}