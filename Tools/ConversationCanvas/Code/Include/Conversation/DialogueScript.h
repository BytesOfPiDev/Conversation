#pragma once

#include "AzCore/Component/EntityId.h"
#include "AzCore/EBus/EBus.h"
#include "AzCore/Name/Name.h"

namespace Conversation
{
    class DialogueScriptRequests : public AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(DialogueScriptRequests);

        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;

        using BusIdType = AZ::EntityId;

        DialogueScriptRequests() = default;
        virtual ~DialogueScriptRequests() = default;

        virtual void RunDialogueScript(AZ::Name nodeId) = 0;
    };

    using DialogueScriptRequestBus = AZ::EBus<DialogueScriptRequests>;

} // namespace Conversation
