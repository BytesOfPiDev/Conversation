#include <ConversationEditor/Nodes/RootNode.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore//Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <ConversationEditor/DataTypes.h>
#include <GraphModel/Integration/Helpers.h>


namespace ConversationEditor
{
	namespace Nodes
	{
		RootNode::RootNode(GraphModel::GraphPtr graph)
			: GraphModel::Node(graph)
		{
			RegisterSlots();
			CreateSlotData();
		}

		RootNode::RootNode()
			: GraphModel::Node()
		{

		}

		RootNode::~RootNode()
		{

		}
		void RootNode::RegisterSlots()
		{
			GraphModel::DataTypePtr stringDataType = GetGraphContext()->GetDataType<AZStd::string>();

			GraphModel::ExtendableSlotConfiguration config;
			config.m_addButtonLabel = "Add Greeting";
			config.m_addButtonTooltip = "";
			config.m_isValid = true;
			config.m_minimumSlots = 1;
			config.m_maximumSlots = 10;

			RegisterSlot(
				GraphModel::SlotDefinition::CreateOutputEvent(
					CommonSlotNames::EXIT, 
					"Greeting", 
					"A greeting to start the conversation.",
					&config));
		}

		void RootNode::Reflect(AZ::ReflectContext* context)
		{
			AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);

			if (serializeContext)
			{
				serializeContext->Class<RootNode, GraphModel::Node>()
					->Version(0)
					;

				if (AZ::EditContext* editContext = serializeContext->GetEditContext())
				{
					editContext->Class<RootNode>("RootNode", "Node that represents a conversation at the highest level.")
						->ClassElement(AZ::Edit::ClassElements::EditorData, "")
						->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "Root")
						;
				}
			}
		}
	} // namespace Nodes
} // namespace ConversationEditor
