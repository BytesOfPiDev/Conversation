#pragma once

#include "AzCore/Console/ILogger.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/std/string/string_view.h"

namespace Conversation
{
    template<typename ComponentType>
    void LOG_EntityComponent(
        AZStd::string_view logTag,
        ComponentType const& dialogueComponent,
        AZStd::string_view logMessage)
    {
        AZLOG( // NOLINT
            logTag.data(),
            "%s on [EntityId: %s | EntityName: %s]: %s.",
            AZ::AzTypeInfo<ComponentType>::Name(),
            dialogueComponent.GetEntityId().ToString().data(),
            dialogueComponent.GetNamedEntityId().GetName().data(),
            logMessage.data());
    }
} // namespace Conversation
