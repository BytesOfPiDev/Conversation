#pragma once

#include <GraphModel/Model/Node.h>

namespace ConversationEditor
{
	namespace Nodes
	{
		/**
		 * Represents a link to a dialogue node within the graph.
		 * 
		 * Links are used to jump between parts of a dialogue tree. This is useful for avoiding
		 * duplicate dialogue options that represent the exact same thing/scenario.
		 * 
		 * A link has no exit function because the conversation continues from whatever dialogue
		 * is jumped to - as if it arrived there through the preceding branches.
		 */
		class Link 
			: public GraphModel::Node
		{
		public:
			AZ_CLASS_ALLOCATOR(Link, AZ::SystemAllocator, 0);
			AZ_RTTI(Link, "{E8260E6A-DACD-4CB0-B691-92B1AD5ECDCD}", GraphModel::Node);

			static void Reflect(AZ::ReflectContext* context);

			struct SlotNames
			{
				inline static const char* LINKEDID = "LinkedId";
			};

			Link() = default;
			Link(GraphModel::GraphPtr graph);
			Link(GraphModel::GraphPtr graph, GraphModel::DataTypePtr dataType);
			~Link() override = default;

		public:
			const char* GetTitle() const override { return "Dialogue Link"; }

		protected:
			void RegisterSlots() override;

		};
	}
}