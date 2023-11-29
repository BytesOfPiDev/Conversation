#pragma once

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <GraphModel/Model/Node.h>

#include "Conversation/ConversationTypeIds.h"

namespace ConversationEditor
{
    namespace Nodes
    {
        class WaypointNode : public GraphModel::Node
        {
        public:
            AZ_RTTI(WaypointNode, WaypointNodeTypeId, GraphModel::Node);
            AZ_CLASS_ALLOCATOR(WaypointNode, AZ::SystemAllocator, 0);
            AZ_DISABLE_COPY_MOVE(WaypointNode);

            static void Reflect(AZ::ReflectContext* context);

            WaypointNode() = default;
            WaypointNode(GraphModel::GraphPtr graph);
            ~WaypointNode() override = default;

            auto GetSubTitle() const -> const char* override
            {
                return "A waypoint";
            }
            auto GetTitle() const -> const char* override
            {
                return "Waypoint";
            }

        protected:
            auto CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition) const -> bool override;
            void RegisterSlots() override;

            void PostLoadSetup(GraphModel::GraphPtr graph, GraphModel::NodeId id) override;
            void PostLoadSetup() override;
        };

    } // namespace Nodes

} // namespace ConversationEditor
