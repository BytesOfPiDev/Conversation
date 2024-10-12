#pragma once

#include "AzCore/RTTI/TypeInfoSimple.h"
#include "GraphModel/Model/Node.h"

#include "Document/NodeRequestBus.h"

namespace ConversationCanvas
{
    /**
     * Represents a link to a dialogue node within the graph.
     *
     * Links are used to jump between parts of a dialogue tree. This is useful
     * for avoiding duplicate dialogue options that represent the exact same
     * thing/scenario.
     *
     * A link has no exit function because the conversation continues from
     * whatever dialogue is jumped to - as if it arrived there through the
     * preceding branches.
     */
    class LinkNode
        : public GraphModel::Node
        , public NodeRequestBus::Handler
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL();
        AZ_TYPE_INFO_WITH_NAME_DECL(LinkNode);
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_DISABLE_COPY_MOVE(LinkNode);

        static void Reflect(AZ::ReflectContext* context);

        LinkNode() = default;
        LinkNode(GraphModel::GraphPtr graph);
        ~LinkNode() override = default;

        auto GetTitle() const -> char const* override
        {
            return "Dialogue Link";
        }

    protected:
        void PostLoadSetup(
            GraphModel::GraphPtr graph, GraphModel::NodeId id) override;
        void PostLoadSetup() override;
        void RegisterSlots() override;

        void UpdateNodeData(DialogueNodeData& nodeData) const override;
    };
} // namespace ConversationCanvas
