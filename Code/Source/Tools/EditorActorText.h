#pragma once

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "AzCore/std/string/string.h"

namespace ConversationEditor
{
    struct EditorActorText
    {
        AZ_TYPE_INFO(
            EditorActorText, "E57F6CA0-9280-435A-A4C3-4A4A277A1A25"); // NOLINT
        AZ_CLASS_ALLOCATOR(EditorActorText, AZ::SystemAllocator, 0); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_text;
    };
} // namespace ConversationEditor
