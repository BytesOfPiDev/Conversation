#pragma once

#include <AzCore/EBus/EBus.h>

namespace Dialogue
{
    class DialogueScriptRequests : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::Crc32 BusIdType;

        virtual ~DialogueScriptRequests() = default;

        virtual void RunDialogueScript()
        {
        }
    };

    using DialogueScriptRequestBus = AZ::EBus<DialogueScriptRequests>;
} // namespace Dialogue