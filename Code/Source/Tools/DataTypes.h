#pragma once

#include "AzCore/Math/Crc.h"
#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

namespace ConversationEditor
{
    AZ_ENUM_CLASS( // NOLINT(*-use-trailing-return-type, *missing-std-forward)
        SlotTypes,
        undefined,
        actor_text,
        dialogue_id,
        dialogue_chunk,
        speaker_tag,
        lua_snippet,
        condition_snippet);

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
} // namespace ConversationEditor

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE( // NOLINT(modernize-use-trailing-return-type)
        ConversationEditor::NodeTypes,
        "{021346A3-C665-4577-BC4C-D540438A0813}");

    AZ_TYPE_INFO_SPECIALIZE( // NOLINT(modernize-use-trailing-return-type)
        ConversationEditor::GeneralSlots,
        "{3EB468A4-96BE-48AD-A397-FC7E06D8CFA3}");

    AZ_TYPE_INFO_SPECIALIZE( // NOLINT(modernize-use-trailing-return-type)
        ConversationEditor::LinkNodeSlots,
        "{920F43A3-71AB-4313-BA6D-4CDC3AEB848C}");

    AZ_TYPE_INFO_SPECIALIZE( // NOLINT(modernize-use-trailing-return-type)
        ConversationEditor::ConditionNodeSlots,
        "{F996C072-DDC3-453E-8086-16239B04514D}");

    AZ_TYPE_INFO_SPECIALIZE( // NOLINT(modernize-use-trailing-return-type)
        ConversationEditor::DialogueNodeSlots,
        "{06BDCFE5-C580-42EB-9A95-4199815F818D}");
} // namespace AZ
