#pragma once

#include "Atom/RPI.Reflect/Image/StreamingImageAsset.h"
#include "AzCore/Component/ComponentBus.h"
#include "AzCore/std/string/string.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    struct DialogueComponentConfig : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI_NO_TYPE_INFO_DECL(); // NOLINT
        AZ_TYPE_INFO_WITH_NAME_DECL(DialogueComponentConfig); // NOLINT
        AZ_CLASS_ALLOCATOR_DECL; // NOLINT
        AZ_DEFAULT_COPY_MOVE(DialogueComponentConfig); // NOLINT

        DialogueComponentConfig() = default;
        ~DialogueComponentConfig() override = default;

        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_speakerIconPath{};
        AZStd::string m_speakerTag{};
        AZStd::string m_displayName{};
    };
} // namespace Conversation
