#include "Tools/Window/Nodes/Waypoint.h"

#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <GraphModel/Integration/Helpers.h>
#include <GraphModel/Model/Common.h>

#include "Tools/DataTypes.h"

namespace ConversationEditor
{
    namespace Nodes
    {
        void WaypointNode::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serialize->Class<WaypointNode, GraphModel::Node>()->Version(0);
                if (AZ::EditContext* editContext = serialize->GetEditContext())
                {
                    editContext
                        ->Class<WaypointNode>(
                            "ActorDialogue", "Represents an actor's response.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(
                            GraphModelIntegration::Attributes::
                                TitlePaletteOverride,
                            "ActorDialogue");
                }
            }
        }

        WaypointNode::WaypointNode(GraphModel::GraphPtr graph)
            : GraphModel::Node(AZStd::move(graph))
        {
            RegisterSlots();
            CreateSlotData();
        }

        auto WaypointNode::CanExtendSlot(
            GraphModel::SlotDefinitionPtr slotDefinition) const -> bool
        {
            return GraphModel::Node::CanExtendSlot(slotDefinition);
        };

        void WaypointNode::RegisterSlots()
        {
            auto const dialogueIdDataType =
                GetGraphContext()->GetDataType(AZ_CRC_CE("dialogue_id"));
            auto input = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input,
                GraphModel::SlotType::Data,
                ToString(NodeAndSlotNames::Waypoint_In),
                "In",
                "",
                GraphModel::DataTypeList{ dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(),
                1,
                1);

            auto output = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Output,
                GraphModel::SlotType::Data,
                ToString(NodeAndSlotNames::Waypoint_Out),
                "Out",
                "",
                GraphModel::DataTypeList{ dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(),
                1,
                1);

            RegisterSlot(input);
            RegisterSlot(output);
        }

        void WaypointNode::PostLoadSetup(
            GraphModel::GraphPtr graph, GraphModel::NodeId id)
        {
            GraphModel::Node::PostLoadSetup(graph, id);
        }

        void WaypointNode::PostLoadSetup()
        {
            GraphModel::Node::PostLoadSetup();
        }
    } // namespace Nodes
} // namespace ConversationEditor
