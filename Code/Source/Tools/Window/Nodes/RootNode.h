#pragma once

#include "GraphModel/Model/Node.h"

namespace ConversationEditor
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
        struct SlotNames
        {
            static constexpr char const* const GreetingOutput = "Greeting";
        };

        struct SlotDisplayNames
        {
            static constexpr char const* const GreetingId = "Greeting Id";
        };

    public:
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_RTTI_NO_TYPE_INFO_DECL(); // NOLINT
        AZ_TYPE_INFO_WITH_NAME_DECL(RootNode); // NOLINT
        AZ_DISABLE_COPY_MOVE(RootNode); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        RootNode() = default; /** Required for serialization */
        /**
         * Creates a RootNode that is added to the graph that is passed in.
         */
        RootNode(GraphModel::GraphPtr graph);
        ~RootNode() override = default;

        auto GetSubTitle() const -> char const* override;
        auto GetTitle() const -> char const* override;

    protected:
        void RegisterSlots() override;
    };
} // namespace ConversationEditor
