#pragma once

#include "Atom/RPI.Reflect/Image/StreamingImageAsset.h"
#include "AzCore/Component/ComponentBus.h"
#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/RTTI.h"
#include "AzCore/std/string/string.h"

#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    struct DialogueComponentConfig : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(DialogueComponentConfig, DialogueComponentConfigTypeId, AZ::ComponentConfig); // NOLINT
        AZ_CLASS_ALLOCATOR(DialogueComponentConfig, AZ::SystemAllocator, 0); // NOLINT
        AZ_DEFAULT_COPY_MOVE(DialogueComponentConfig); // NOLINT

        DialogueComponentConfig() = default;
        ~DialogueComponentConfig() override = default;

        static void Reflect(AZ::ReflectContext* context);

        AZStd::vector<AZ::Data::Asset<Conversation::ConversationAsset>> m_assets{};
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_speakerIconPath{};
        AZStd::string m_speakerTag{};
        AZStd::string m_displayName{};
    };
} // namespace Conversation
