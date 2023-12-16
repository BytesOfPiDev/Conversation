#pragma once

#include "Conversation/ConversationTypeIds.h"
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
        class Link : public GraphModel::Node
        {
        public:
            AZ_CLASS_ALLOCATOR(Link, AZ::SystemAllocator, 0); // NOLINT
            AZ_RTTI(Link, LinkNodeTypeId, GraphModel::Node); // NOLINT

            static void Reflect(AZ::ReflectContext* context);

            struct SlotNames
            {
                static constexpr char const* LINKEDID = "LinkedId";
            };

            Link() = default;
            Link(Link const&) = delete;
            Link(Link&&) = delete;
            auto operator=(Link const&) -> Link& = delete;
            auto operator=(Link&&) -> Link& = delete;
            Link(GraphModel::GraphPtr graph);
            ~Link() override = default;

        public:
            auto GetTitle() const -> char const* override
            {
                return "Dialogue Link";
            }

        protected:
            void RegisterSlots() override;
        };
    } // namespace Nodes
} // namespace ConversationEditor
