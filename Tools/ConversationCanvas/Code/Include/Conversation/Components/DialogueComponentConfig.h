#pragma once

#include "Atom/RPI.Reflect/Image/StreamingImageAsset.h"
#include "AzCore/Component/ComponentBus.h"
#include "AzCore/Script/ScriptAsset.h"
#include "AzCore/std/string/string.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    /**
     * DialogueComponent configuration options.
     *
     * These options are used to configure an entity's dialogue component. The
     * conversation system will use these settings to setup the entity's
     * dialogue during entity activation, displaying information about the
     * entity, and as a general description of the entity.
     */
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

        /** An icon that represents the entity. */
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_speakerIconPath{};
        AZ::Data::Asset<AZ::ScriptAsset> m_companionScript{};
        /** A tag that can be used to identify the entity. */
        AZStd::string m_speakerTag{};
        /** The entity's display name*/
        AZStd::string m_displayName{};
    };
} // namespace Conversation
