#include "Tools/Window/Nodes/Condition.h"
#include "AzCore/RTTI/RTTIMacros.h"

#include <AzCore/Serialization/SerializeContext.h>

#include <utility>

namespace ConversationEditor
{

    namespace Nodes
    {
        void ConditionNode::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serialize->Class<ConditionNode, GraphModel::Node>()->Version(0);
            }
        }

        ConditionNode::ConditionNode(GraphModel::GraphPtr graph)
            : GraphModel::Node(AZStd::move(graph))
        {
        }
        auto ConditionNode::CanExtendSlot(
            GraphModel::SlotDefinitionPtr slotDefinition) const -> bool
        {
            return false;
        };

        void ConditionNode::RegisterSlots()
        {
        }

        void ConditionNode::PostLoadSetup(
            GraphModel::GraphPtr graph, GraphModel::NodeId id)
        {
            GraphModel::Node::PostLoadSetup(graph, id);
        }

        void ConditionNode::PostLoadSetup()
        {
            GraphModel::Node::PostLoadSetup();
        }
    } // namespace Nodes
} // namespace ConversationEditor
