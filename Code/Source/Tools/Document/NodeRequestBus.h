#pragma once

#include "AzCore/EBus/EBus.h"
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

        virtual void UpdateNodeData(DialogueNodeData& /*nodeData*/) const {};
    };

    using NodeRequestBus = AZ::EBus<NodeRequests>;
} // namespace ConversationCanvas
