#pragma once

#include "AzCore/Math/Crc.h"
#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/std/string/string.h"

namespace ConversationCanvas
{
    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        SlotTypes,
        undefined,
        actor_text,
        dialogue_id,
        dialogue_chunk,
        speaker_tag,
        lua_snippet,
        condition_snippet,
        sound_asset,
        audio_control);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        NodeTypes,
        Undefined,
        Condition,
        Dialogue,
        Link);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        GeneralSlots,
        NodeName);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        LinkNodeSlots,
        in_from,
        in_to);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        ConditionNodeSlots,
        out_condition);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        DialogueScriptSlots,
        out_chunk);

    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        DialogueNodeSlots,
        in_comment,
        in_condition,
        in_isStarter,
        in_name,
        in_parent,
        in_speakerTag,
        in_shortText,
        out_id);

    template<typename T>
    constexpr auto ToTag(T const type) -> auto
    {
        static_assert(AZStd::is_enum_v<T>, "The type must be an enum!");
        return AZ::Crc32(ToString(type));
    }
} // namespace ConversationCanvas

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(
        ConversationCanvas::NodeTypes,
        "{021346A3-C665-4577-BC4C-D540438A0813}");

    AZ_TYPE_INFO_SPECIALIZE(
        ConversationCanvas::GeneralSlots,
        "{3EB468A4-96BE-48AD-A397-FC7E06D8CFA3}");

    AZ_TYPE_INFO_SPECIALIZE(
        ConversationCanvas::LinkNodeSlots,
        "{920F43A3-71AB-4313-BA6D-4CDC3AEB848C}");

    AZ_TYPE_INFO_SPECIALIZE(
        ConversationCanvas::ConditionNodeSlots,
        "{F996C072-DDC3-453E-8086-16239B04514D}");

    AZ_TYPE_INFO_SPECIALIZE(
        ConversationCanvas::DialogueNodeSlots,
        "{06BDCFE5-C580-42EB-9A95-4199815F818D}");
} // namespace AZ
