#pragma once

#include "AzCore/Console/ILogger.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/std/string/string_view.h"

namespace Conversation
{
    template<typename ComponentType>
    void LOGTAG_EntityComponent(
        [[maybe_unused]]AZStd::string_view logTag,
        ComponentType const& component,
        AZStd::string_view logMessage)
    {
        AZLOG( // NOLINT
            logTag.data(),
            "%s on [EntityId: %s | EntityName: %s]: %s.",
            AZ::AzTypeInfo<ComponentType>::Name(),
            component.GetEntityId().ToString().data(),
            component.GetNamedEntityId().GetName().data(),
            logMessage.data());
    }
} // namespace Conversation
