#include <ConversationEditor/ConversationGraph.h>

#include <AssetBuilderSDK/AssetBuilderSDK.h>

#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Serialization/EditContext.h>

#include <Conversation/ConversationBus.h>

#include <ConversationEditor/DataTypes.h>
#include <ConversationEditor/Nodes/ActorDialogue.h>
#include <ConversationEditor/Nodes/Link.h>
#include <ConversationEditor/Nodes/RootNode.h>

#include <GraphModel/Model/Connection.h>

namespace ConversationEditor
{
	void ConversationGraph::Reflect(AZ::ReflectContext* context)
	{
		if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serializeContext->Class<ConversationGraph, GraphModel::Graph>()
				->Version(0)
				;

			if (AZ::EditContext* editContext = serializeContext->GetEditContext())
			{
				editContext->Class<ConversationGraph>("DialogueGraph", "A dialogue graph.")
					;
			}
		}
	}

	ConversationGraph::ConversationGraph(GraphModel::GraphContextPtr graphContext)
		: GraphModel::Graph(graphContext)
	{

	}

	GraphModel::NodePtr ConversationGraph::FindRoot()
	{
		const GraphModel::Graph::NodeMap nodemap = GetNodes();
		auto iter = std::find_if(nodemap.begin(), nodemap.end(), [](AZStd::pair<GraphModel::NodeId, GraphModel::NodePtr> nodepair)
			{
				return nodepair.second->RTTI_IsTypeOf(AZ::AzTypeInfo<ConversationEditor::Nodes::RootNode>::Uuid());
			});

		if (iter == nodemap.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	AZStd::shared_ptr<Conversation::ConversationAsset> ConversationGraph::MakeAsset()
	{
		GraphModel::NodePtr rootNode = FindRoot();

		if (!rootNode)
		{
			return nullptr;
		}

		auto newConversationAsset = AZStd::make_shared<Conversation::ConversationAsset>();

		for (auto nodePair : GetNodes())
		{
			if (nodePair.second.get()->RTTI_IsTypeOf(ConversationEditor::Nodes::Link::TYPEINFO_Uuid()))
			{
				ProcessLinkNode(newConversationAsset, nodePair.second);
			}
			else if (nodePair.second.get()->RTTI_IsTypeOf(ConversationEditor::Nodes::ActorDialogue::TYPEINFO_Uuid()))
			{
				ProcessDialogueNode(newConversationAsset, nodePair.second);
			}
		}

		/**
		 * IMPORTANT: This should be done AFTER adding all dialogues to the asset. This is
		 * because the dialogue asset will only add a starting ID if it can find a dialogue
		 * with a matching ID in order to guarantee its existence.
		 */
		for (auto slotPtr : rootNode->GetExtendableSlots(CommonSlotNames::EXIT))
		{
			for (auto connection : slotPtr->GetConnections())
			{
				GraphModel::ConstSlotPtr dialogueDataSlot = connection->GetTargetNode()->GetSlot(CommonSlotNames::DIALOGUEDATA);
				if (!dialogueDataSlot)
				{
					AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Failed to find dialogue data slot on a root node's exit target.\n");
					continue;
				}

				AZStd::any dialogueDataSlotValue = dialogueDataSlot->GetValue();

				if (!dialogueDataSlotValue.is<Conversation::DialogueDataPtr>())
				{
					continue;
				}

				auto dialogueDataPtr = dialogueDataSlot->GetValue<Conversation::DialogueDataPtr>();
				newConversationAsset->AddStartingId(dialogueDataPtr->GetId());
				AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Added starting ID: %s.\n", dialogueDataPtr->GetId().ToString<AZStd::string>().c_str());
			}
		}

		return newConversationAsset;
	}

	void ConversationGraph::ProcessLinkNode(AZStd::shared_ptr<Conversation::ConversationAsset> ConversationAsset, const GraphModel::NodePtr linkNode)
	{
		/**
		 * @todo Improve error checking. The should be only one connection to the DialogueId node.
		 * I've implemented some logic to check for it, but it's untested. It's untested
		 * because I'm still figuring out how linking is going to work.
		 */
		GraphModel::Slot::ConnectionList enterConnectionList =
			linkNode->GetSlot(CommonSlotNames::ENTER)->GetConnections();

		if (!enterConnectionList.empty())
		{
			// The dialogue that we need to update with the linked dialogue response.
			auto parentDialogueDataPtr = (*enterConnectionList.begin())
				->GetSourceNode()->GetSlot(CommonSlotNames::DIALOGUEDATA)->GetValue<Conversation::DialogueDataPtr>();
			AZ_Error("DialogueGraph", parentDialogueDataPtr != nullptr, "Parent dialogue is a null pointer.");

			GraphModel::Slot::ConnectionList dialogueIdPropertyConnectionList =
				linkNode->GetSlot(Nodes::Link::SlotNames::LINKEDID)->GetConnections();

			AZ_Error("DialogueGraph", dialogueIdPropertyConnectionList.size() == 1,
				"Multiple connections to a Link's DialogueId property detected. There should only ever be a single connection.");
			if (!dialogueIdPropertyConnectionList.empty())
			{
				// The dialogue to add a possible response to the parent dialogue.
				auto responseDialogueDataPtr = (*dialogueIdPropertyConnectionList.begin())
					->GetSourceNode()->GetSlot(CommonSlotNames::DIALOGUEDATA)->GetValue<Conversation::DialogueDataPtr>();
				AZ_Error("DialogueGraph", responseDialogueDataPtr != nullptr, "Response dialogue is a null pointer.");

				parentDialogueDataPtr->AddResponseId(responseDialogueDataPtr->GetId());
				/**
				 * The nodes in the dialogue graph are not guaranteed to process in a particularly order.
				 * Even though each node has a pointer to its dialogue data, when a node is processed,
				 * the dialogue data is COPIED into a new object on the stack. As a result, if the parent
				 * dialogue has already been processed, updating the pointer will not reflect any changes
				 * to the dialogue asset. As a result, we tell the dialogue asset to add this as a response
				 * to the parent ID. If the parent ID doesn't yet exist in the asset, the asset ignores
				 * the request.
				 */
				ConversationAsset->AddResponseToDialogue(parentDialogueDataPtr->GetId(), responseDialogueDataPtr->GetId());
			}
		}
	}

	void ConversationGraph::ProcessDialogueNode(AZStd::shared_ptr<Conversation::ConversationAsset> ConversationAsset, const GraphModel::NodePtr actorNode)
	{
		if (!actorNode || !ConversationAsset)
		{
			return;
		}

		GraphModel::SlotPtr dialogueDataSlot = actorNode->GetSlot(CommonSlotNames::DIALOGUEDATA);
		if (!dialogueDataSlot)
		{
			return;
		}

		AZStd::any dialogueDataSlotValue = dialogueDataSlot->GetValue();
		if (!dialogueDataSlotValue.is<Conversation::DialogueDataPtr>())
		{
			return;
		}

		auto dialogueDataPtr = dialogueDataSlot->GetValue<const Conversation::DialogueDataPtr>();
		if (!dialogueDataPtr)
		{
			return;
		}

		/**
		 * We get a copy of the dialogue data, instead of a pointer because
		 * we are going to edit some of the information before adding it
		 * to the dialogue asset.
		 */
		Conversation::DialogueData dialogueData = *dialogueDataPtr;

		const GraphModel::Node::ExtendableSlotSet& exitSlots = actorNode->GetExtendableSlots(CommonSlotNames::EXIT);
		for (GraphModel::ConstSlotPtr slot : exitSlots)
		{
			for (auto slotConnection : slot->GetConnections())
			{
				auto responseActorDialogueNode = slotConnection->GetTargetNode();
				if (responseActorDialogueNode->RTTI_IsTypeOf(ConversationEditor::Nodes::ActorDialogue::TYPEINFO_Uuid()))
				{
					GraphModel::SlotPtr responseActorDialogueDataSlot = responseActorDialogueNode->GetSlot(CommonSlotNames::DIALOGUEDATA);
					AZStd::any responseActorDialogueDataSlotValue = responseActorDialogueDataSlot->GetValue();
					if (!responseActorDialogueDataSlotValue.is<Conversation::DialogueDataPtr>())
					{
						continue;
					}

					auto responseDialogueDataPtr = responseActorDialogueDataSlot->GetValue<Conversation::DialogueDataPtr>();
					dialogueData.AddResponseId(responseDialogueDataPtr->GetId());
					AZ_TracePrintf(AssetBuilderSDK::InfoWindow, "Response ID Added: %s.\n", responseDialogueDataPtr->GetId().ToString<AZStd::string>().c_str());
				}
			}
		}

		ConversationAsset->AddDialogue(dialogueData);
	}
}
