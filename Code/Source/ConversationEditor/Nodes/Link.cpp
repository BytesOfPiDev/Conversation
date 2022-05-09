#include <ConversationEditor/Nodes/Link.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/string/string.h>
#include <Conversation/DialogueData.h>
#include <ConversationEditor/DataTypes.h>
#include <GraphModel/Integration/Helpers.h>

namespace ConversationEditor
{
	namespace Nodes
	{
		Link::Link(GraphModel::GraphPtr graph)
			: GraphModel::Node(graph)
		{
			RegisterSlots();
			CreateSlotData();
		}

		Link::Link(GraphModel::GraphPtr graph, GraphModel::DataTypePtr dataType)
			: Link(graph)
		{

		}

		void Link::Reflect(AZ::ReflectContext* context)
		{
			if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
			{
				serializeContext->Class<Link, GraphModel::Node>()
					->Version(0)
					;

				if (AZ::EditContext* editContext = serializeContext->GetEditContext())
				{
					editContext->Class<Link>("Link", "Note that represent branching off to a dialogue defined elsewhere.")
						->ClassElement(AZ::Edit::ClassElements::EditorData, "")
						->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "Link")
						;
				}
			}
		}

		void Nodes::Link::RegisterSlots()
		{
			GraphModel::DataTypePtr uniqueIdDataType = GetGraphContext()->GetDataType<AZ::Uuid>();

			RegisterSlot(
				GraphModel::SlotDefinition::CreateInputEvent(
					CommonSlotNames::ENTER,
					CommonSlotDisplayNames::ENTER, 
					CommonSlotDescriptions::ENTER));

			RegisterSlot(
				GraphModel::SlotDefinition::CreateInputData(
					SlotNames::LINKEDID,
					"Dialogue Id",
					GetGraphContext()->GetDataType<Conversation::DialogueId>(),
					AZStd::make_any<Conversation::DialogueId>(),
					"The unique ID of an existing dialogue within this graph."));
		}
	} // namespace Nodes
} // namespace ConversationEditor
