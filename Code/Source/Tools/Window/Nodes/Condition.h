#pragma once

#include "Conversation/ConversationTypeIds.h"
#include "GraphModel/Model/Common.h"
#include <GraphModel/Model/Node.h>

namespace ConversationEditor
{
    namespace Nodes
    {
        class ConditionNode : public GraphModel::Node
        {
        public:
            AZ_RTTI(
                ConditionNode, ConditionNodeTypeId, GraphModel::Node); // NOLINT
            AZ_CLASS_ALLOCATOR(ConditionNode, AZ::SystemAllocator, 0); // NOLINT
            AZ_DISABLE_COPY_MOVE(ConditionNode); // NOLINT

            static void Reflect(AZ::ReflectContext* context);

            ConditionNode() = default;
            ConditionNode(GraphModel::GraphPtr graph);
            ~ConditionNode() override = default;

            auto GetSubTitle() const -> char const* override
            {
                return "Conditional requirement";
            }

            auto GetTitle() const -> char const* override
            {
                return "Condition";
            }

        protected:
            auto CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition)
                const -> bool override;
            void RegisterSlots() override;

            void PostLoadSetup(
                GraphModel::GraphPtr graph, GraphModel::NodeId id) override;
            void PostLoadSetup() override;
        };
    } // namespace Nodes
} // namespace ConversationEditor
