#include <ConversationEditor/Nodes/ActorDialogue.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>

#include <ConversationEditor/DataTypes.h>

#include <Conversation/DialogueData.h>

#include <GraphModel/Integration/Helpers.h>
#include <GraphModel/Model/GraphContext.h>

namespace ConversationEditor
{
	namespace Nodes
	{
		void ActorDialogue::Reflect(AZ::ReflectContext* context)
		{
			AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
			if (serializeContext)
			{
				serializeContext->Class<ActorDialogue, GraphModel::Node>()
					->Version(0)
					;

				if (AZ::EditContext* editContext = serializeContext->GetEditContext())
				{
					editContext->Class<ActorDialogue>("ActorDialogue", "Represents a actor's response.")
						->ClassElement(AZ::Edit::ClassElements::EditorData, "")
						->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "ActorDialogue")
						;
				}
			}
		}

		ActorDialogue::ActorDialogue(GraphModel::GraphPtr graph)
			: GraphModel::Node(graph)
		{
			RegisterSlots();
			CreateSlotData();
		}
		bool ActorDialogue::CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition) const
		{
			return GraphModel::Node::CanExtendSlot(slotDefinition);
		}
		void ActorDialogue::RegisterSlots()
		{
			{
				GraphModel::ExtendableSlotConfiguration config;
				config.m_addButtonLabel = "Add Dialogue";
				config.m_addButtonTooltip = "Add dialogue that the current dialogue can potentially branch to.";
				config.m_isValid = true;
				config.m_minimumSlots = 0;
				config.m_maximumSlots = 10;

				RegisterSlot(
					GraphModel::SlotDefinition::CreateOutputEvent(
						CommonSlotDisplayNames::EXIT,
						CommonSlotDisplayNames::EXIT,
						CommonSlotDescriptions::EXIT,
						&config));
			}

			RegisterSlot(
				GraphModel::SlotDefinition::CreateInputEvent(
					CommonSlotNames::ENTER,
					CommonSlotDisplayNames::ENTER,
					CommonSlotDescriptions::ENTER));

			auto dialogueDataPtr = AZStd::make_shared<Conversation::DialogueData>(true);
			RegisterSlot(
				GraphModel::SlotDefinition::CreateProperty(
					CommonSlotNames::DIALOGUEID,
					CommonSlotDisplayNames::DIALOGUEID,
					GetGraphContext()->GetDataType<Conversation::DialogueId>(),
					AZStd::any(dialogueDataPtr->GetId()),
					CommonSlotDescriptions::DIALOGUEID));

			RegisterSlot(
				GraphModel::SlotDefinition::CreateProperty(
					CommonSlotNames::DIALOGUEDATA,
					CommonSlotDisplayNames::DIALOGUEDATA,
					GetGraphContext()->GetDataType<Conversation::DialogueDataPtr>(),
					AZStd::any(dialogueDataPtr),
					CommonSlotDescriptions::DIALOGUEDATA)
			);

			RegisterSlot(
				GraphModel::SlotDefinition::CreateOutputData(
					CommonSlotNames::OUTPUTDIALOGUEID,
					CommonSlotDisplayNames::OUTPUTDIALOGUEID,
                            GetGraphContext()->GetDataType<Conversation::DialogueId>(),
					CommonSlotDescriptions::OUTPUTDIALOGUEID));
		}
	}
}