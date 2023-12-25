#pragma once

#include "AzCore/Preprocessor/Enum.h"
#include "AzCore/RTTI/TypeInfoSimple.h"

namespace ConversationEditor
{
    constexpr auto ActorTextDataTypeName = "actor_text";
    constexpr auto CommentDataTypeName = "comment";
    constexpr auto DialogueIdTypeName = "dialogue_id";
    constexpr auto DialogueScriptDataTypeName = "dialogue_script";
    constexpr auto StringDataTypeName = "string";

    AZ_ENUM( // NOLINT
        NodeTypes,
        Undefined,
        Condition,
        Dialogue,
        Link);

    AZ_ENUM( // NOLINT
        GeneralSlots,
        NodeName);

    AZ_ENUM( // NOLINT
        LinkNodeSlots,
        inFrom,
        inTo);

    AZ_ENUM_CLASS( // NOLINT
        ConditionNodeSlots,
        outCondition);

    AZ_ENUM_CLASS( // NOLINT
        DialogueNodeSlots,
        inCondition,
        inIsStarter,
        inName,
        inParent,
        inSpeaker,
        inShortText,
        outDialogue);
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
