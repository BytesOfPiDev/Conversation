#include "Window/Nodes/Link.h"

#include "AtomToolsFramework/Graph/GraphCompiler.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "AzCore/std/string/string.h"
#include "GraphModel/Integration/Helpers.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/Connection.h"
#include "GraphModel/Model/Slot.h"

#include "Conversation/DialogueData.h"
#include "ConversationCanvasTypeIds.h"
#include "DataTypes.h"
#include "Document/NodeRequestBus.h"

namespace ConversationCanvas
{
    AZ_RTTI_NO_TYPE_INFO_IMPL(LinkNode, GraphModel::Node, NodeRequests);
    AZ_TYPE_INFO_WITH_NAME_IMPL(LinkNode, "LinkNode", LinkNodeTypeId);
    AZ_CLASS_ALLOCATOR_IMPL(LinkNode, AZ::SystemAllocator);

    LinkNode::LinkNode(GraphModel::GraphPtr graph)
        : GraphModel::Node(AZStd::move(graph))
    {
        RegisterSlots();
        CreateSlotData();
    }

    void LinkNode::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext =
                azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<LinkNode, GraphModel::Node, NodeRequests>()
                ->Version(0);

            if (AZ::EditContext* editContext =
                    serializeContext->GetEditContext())
            {
                editContext
                    ->Class<LinkNode>(
                        "Link",
                        "Node that links/jumps from one dialogue to another.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        GraphModelIntegration::Attributes::TitlePaletteOverride,
                        "LinkDataColorPalette");
            }
        }
    }

    void LinkNode::PostLoadSetup(
        GraphModel::GraphPtr graph, GraphModel::NodeId id)
    {
        GraphModel::Node::PostLoadSetup(graph, id);
    }

    void LinkNode::PostLoadSetup()
    {
        GraphModel::Node::PostLoadSetup();
    }

    void LinkNode::RegisterSlots()
    {
        GraphModel::DataTypePtr uniqueIdDataType =
            GetGraphContext()->GetDataType<AZ::Uuid>();
        auto dialogueIdDataType{ GetGraphContext()->GetDataType(
            ToTag(SlotTypes::dialogue_id)) };

        auto to = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDefinition(
                GraphModel::SlotDirection::Input,
                GraphModel::SlotType::Data,
                ToString(LinkNodeSlots::in_from),
                "From",
                "The node to link from.",
                { dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(),
                {},
                {},
                {},
                {},
                {},
                true,
                false));

        auto from = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDefinition(
                GraphModel::SlotDirection::Input,
                GraphModel::SlotType::Data,
                ToString(LinkNodeSlots::in_to),
                "To",
                "The node to link to.",
                { dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(),
                {},
                {},
                {},
                {},
                {},
                true,
                false));

        RegisterSlot(AZStd::move(from));
        RegisterSlot(AZStd::move(to));
    }

    void LinkNode::UpdateNodeData(DialogueNodeData& nodeData) const
    {
        if (AtomToolsFramework::GraphCompiler::IsCompileLoggingEnabled())
        {
            AZLOG_INFO("Updating link node...");
        }

        auto const fromSlot = GetSlot(ToString(LinkNodeSlots::in_from));

        auto toSlot = GetSlot(ToString(LinkNodeSlots::in_to));

        // The connection to our 'From' input slot
        GraphModel::ConstConnectionPtr const fromSlotConnection =
            [fromSlot, thisNode = azrtti_cast<GraphModel::Node const*>(this)]()
            -> GraphModel::ConstConnectionPtr
        {
            auto const fromSlotConnections = fromSlot->GetConnections();

            if (fromSlotConnections.empty() || fromSlotConnections.size() != 1)
            {
                return nullptr;
            }

            GraphModel::ConstConnectionPtr const fromSlotConnection =
                fromSlotConnections.front();
            // The source should not be us, we should be the target
            if (fromSlotConnection->GetSourceNode().get() == thisNode)
            {
                return nullptr;
            }

            return fromSlotConnection;
        }();

        // Finds the connection to our 'To' input slot
        GraphModel::ConstConnectionPtr const toSlotConnection =
            [toSlot, thisNode = azrtti_cast<GraphModel::Node const*>(this)]()
            -> GraphModel::ConstConnectionPtr
        {
            auto const toSlotConnections = toSlot->GetConnections();

            if (toSlotConnections.empty() || toSlotConnections.size() != 1)
            {
                return nullptr;
            }

            GraphModel::ConstConnectionPtr const toSlotConnection =
                toSlotConnections.front();

            // The source should not be us, we should be the target
            if (toSlotConnection->GetSourceNode().get() == thisNode)
            {
                AZ_Error(
                    "LinkNode",
                    false,
                    "Invalid slot configuration. The source node should not be "
                    "us!");

                return nullptr;
            }

            return toSlotConnection;
        }();

        if (!fromSlotConnection || !toSlotConnection)
        {
            AZLOG_WARN("Link node slot connection issue");
            return;
        }

        nodeData.m_linkData.m_from = fromSlotConnection->GetSourceNode();
        nodeData.m_linkData.m_to = toSlotConnection->GetSourceNode();
    }

} // namespace ConversationCanvas
