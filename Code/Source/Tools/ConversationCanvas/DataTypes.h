#pragma once

#include "AzCore/Math/Crc.h"
#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/RTTI/TypeInfoSimple.h"
#include "AzCore/std/string/string.h"

namespace ConversationCanvas
{
    AZ_ENUM_CLASS(
        GraphValueType,
        undefined,
        actor_text,
        audio_control,
        dialogue_chunk,
        dialogue_id,
        lua_condition_function,
        lua_condition_snippet,
        lua_function,
        lua_function_name,
        lua_snippet,
        sound_asset,
        speaker_tag);

    AZ_ENUM_CLASS(NodeTypes, Undefined, Condition, Dialogue, Link);

    AZ_ENUM_CLASS(GeneralSlots, NodeName);

    AZ_ENUM_CLASS(LinkNodeSlots, in_from, in_to);

    AZ_ENUM_CLASS(ConditionNodeSlots, out_condition);

    AZ_ENUM_CLASS(DialogueScriptSlots, out_chunk);

    AZ_ENUM_CLASS(
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

    inline auto IsSlotType(
        GraphModel::ConstSlotPtr const& slot, GraphValueType const slotType)
        -> bool
    {
        return slot->GetDataType()->GetTypeEnum() == ToTag(slotType);
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
