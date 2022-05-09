#pragma once

#include <GraphModel/Model/Common.h>
#include <GraphModel/Model/Graph.h>

#include <Conversation/ConversationAsset.h>

namespace ConversationEditor
{
	class ConversationGraph
		: public GraphModel::Graph
	{
	public:
		AZ_CLASS_ALLOCATOR(ConversationGraph, AZ::SystemAllocator, 0);
		AZ_RTTI(ConversationGraph, "{D446D406-C480-45E6-81AD-52027B02C65C}", GraphModel::Graph);
		static void Reflect(AZ::ReflectContext* context);

		ConversationGraph() = default;
		ConversationGraph(const ConversationGraph&) = delete;
		~ConversationGraph() override = default;

		explicit ConversationGraph(GraphModel::GraphContextPtr graphContext);

		[[nodiscard]] GraphModel::NodePtr FindRoot();

		[[nodiscard]] AZStd::shared_ptr<Conversation::ConversationAsset> MakeAsset();

	private:
		void ProcessLinkNode(AZStd::shared_ptr<Conversation::ConversationAsset> ConversationAsset, const GraphModel::NodePtr linkNode);
		void ProcessDialogueNode(AZStd::shared_ptr<Conversation::ConversationAsset> ConversationAsset, const GraphModel::NodePtr actorNode);
	};
}
