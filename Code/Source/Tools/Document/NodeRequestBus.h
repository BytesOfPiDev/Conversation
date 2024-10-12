#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/RTTI/RTTIMacros.h"

#include "GraphModel/Model/Common.h"
#include "Tools/NodeData.h"

namespace ConversationCanvas
{
    class NodeRequests : public AZ::EBusTraits
    {
    public:
        AZ_RTTI(NodeRequests, "{0F3E8101-2EE8-42EF-88F9-BFB3F2684290}");
        AZ_DISABLE_COPY_MOVE(NodeRequests);

        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        NodeRequests() = default;
        virtual ~NodeRequests() = default;

        virtual void Update(GraphModel::ConstNodePtr){};
        virtual void UpdateNodeData(DialogueNodeData& /*nodeData*/) const {};
    };

    struct NodeRequestBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::TypeId;
    };

    using NodeRequestBus = AZ::EBus<NodeRequests, NodeRequestBusTraits>;
} // namespace ConversationCanvas
