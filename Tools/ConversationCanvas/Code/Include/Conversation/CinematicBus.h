#pragma once

#include "AzCore/EBus/EBus.h"
#include "AzCore/Interface/Interface.h"
#include "AzCore/Name/Name.h"

namespace Conversation
{
    using CinematicId = AZ::Name;

    /**
     * Requests needed to handle cinematics during a conversation.
     */
    class CinematicRequests
    {
    public:
        AZ_DISABLE_COPY_MOVE(CinematicRequests);

        CinematicRequests() = default;
        virtual ~CinematicRequests() = default;

        virtual void StartCinematic(CinematicId cinematicTag) = 0;
    };

    struct CinematicRequestBusTraits : AZ::EBusTraits
    {
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;

        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;
    };

    using CinematicRequestBus =
        AZ::EBus<CinematicRequests, CinematicRequestBusTraits>;

    using CinematicInterface = AZ::Interface<CinematicRequests>;

    /**
     * Notifications that we conversation will try to act upon when received.
     */
    class CinematicNotifications : AZ::EBusTraits
    {
    public:
        AZ_DISABLE_COPY_MOVE(CinematicNotifications);

        CinematicNotifications() = default;
        virtual ~CinematicNotifications() = default;

        static constexpr AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::Single;

        using BusIdType = CinematicId;

        virtual void OnCinematicFinished() = 0;
    };

    using CinematicNotificationBus = AZ::EBus<CinematicNotifications>;
} // namespace Conversation
