#pragma once

#include <GraphModel/Model/Node.h>

#include "Conversation/ConversationTypeIds.h"

namespace ConversationEditor
{
    namespace Nodes
    {
        /**
         * The first node of a dialogue tree.
         *
         * This node holds the list of greetings that an NPC can use at the start of a conversation.
         *
         * @todo Implement restricting only one of these per graph. Possibly by having it present be default.
         * @todo Add additional information that describes the dialogue tree as a whole, such as comments.
         */
        class RootNode : public GraphModel::Node
        {
            static char const* const RootNodeTitle;
            static char const* const RootNodeSubTitle;

            struct SlotNames
            {
                static constexpr char const* const GreetingOutput = "Greeting";
            };

            struct SlotDisplayNames
            {
                static constexpr char const* const GreetingId = "Greeting Id";
            };

        public:
            AZ_CLASS_ALLOCATOR(RootNode, AZ::SystemAllocator, 0); // NOLINT
            AZ_RTTI(RootNode, RootNodeTypeId, Node); // NOLINT

            static void Reflect(AZ::ReflectContext* context);

            RootNode() = default; /** Required for serialization */
            /**
             * Creates a RootNode that is added to the graph that is passed in.
             */
            RootNode(GraphModel::GraphPtr graph);
            ~RootNode() override = default;

            auto GetSubTitle() const -> const char* override
            {
                return RootNodeSubTitle;
            }
            auto GetTitle() const -> const char* override
            {
                return RootNodeTitle;
            }

        protected:
            void RegisterSlots() override;
        };
    } // namespace Nodes
} // namespace ConversationEditor
