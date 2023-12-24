#pragma once

#include <AzCore/Preprocessor/Enum.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <GraphModel/Model/DataType.h>
#include <GraphModel/Model/Slot.h>

namespace ConversationEditor
{
    constexpr auto StringDataTypeName = "string";
    constexpr auto ActorTextDataTypeName = "actor_text";
    constexpr auto CommentDataTypeName = "comment";
    constexpr auto DialogueIdTypeName = "dialogue_id";

    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    AZ_ENUM_CLASS(
        NodeAndSlotNames,
        Undefined,
        // Properties common to all nodes
        NodeIdProperty,
        NodeNameProperty,
        CommentProperty,
        // Root Node
        ConversationRoot,
        ConversationRootOutput_GreetingSlot,
        // Actor Dialogue Node
        ActorDialogue,
        ActorDialogueEvent_Enter,
        ActorDialogueEvent_Exit,
        ActorDialogueInput_ParentId,
        ActorDialogueInput_ConditionId,
        ActorDialogueOutput_Id,
        ActorDialogueProperty_Text,
        ActorDialogueProperty_Script,
        ActorDialogueProperty_Speaker,
        ActorDialogueProperty_DialogueId,
        LinkNode,
        LinkNodeInput_LinkFrom,
        LinkNodeInput_LinkTo,
        Condition,
        Waypoint,
        Waypoint_In,
        Waypoint_Out);

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
        inParent,
        inIsStarter);

    namespace CommonSlotDisplayNames
    {
        inline constexpr auto AVAILABILITYSCRIPT = "ActiveScript";
        inline constexpr char const* ActorText = "Text";
        inline constexpr char const* DialogueId = "Id";
        static char const* DIALOGUEDATA = "DialogueData";
        static char const* DIALOGUESCRIPT = "DialogueScriptId";
        static char const* ENTER = "Enter";
        static char const* EXIT = "Exit";
        inline constexpr char const* OutputDialogueId = "DialogueId";
        inline constexpr char const* ResponseTo = "Response To";
    } // namespace CommonSlotDisplayNames

    namespace CommonSlotNames
    {
        inline constexpr char const* AVAILABILITYSCRIPT = "AvailabilityScript";
        inline constexpr char const* DIALOGUEDATA = "DialogueData";
        inline constexpr char const* DIALOGUESCRIPT = "DialogueScript";
        inline constexpr char const* ENTER = "Enter";
        inline constexpr char const* EXIT = "Exit";
        inline constexpr char const* OutputDialogueId = "OutputDialogueId";
        inline constexpr char const* ResponseTo = "ResponseTo";
    } // namespace CommonSlotNames

    namespace CommonSlotDescriptions
    {
        constexpr auto ActorText = "The text that will show up on screen.";
        constexpr auto DialogueId =
            "The globally unique identifier for this specific dialogue.";
        constexpr auto DIALOGUEDATA =
            "Stores data related to a specific dialogue node.";
        constexpr auto DIALOGUESCRIPT =
            "An identifier for a script that runs when the dialogue is used.";
        constexpr auto ENTER = "";
        constexpr auto EXIT = "";
        constexpr auto OutputDialogueId = DialogueId;
        constexpr auto ResponseTo = "The node that this is a response to.";
    } // namespace CommonSlotDescriptions

} // namespace ConversationEditor

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(
        ConversationEditor::NodeAndSlotNames,
        "{543AB94D-202A-4210-8987-9CBD8668BC76}"); // NOLINT
}
