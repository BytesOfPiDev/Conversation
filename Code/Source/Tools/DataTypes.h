#pragma once

#include <AzCore/Preprocessor/Enum.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <GraphModel/Model/DataType.h>
#include <GraphModel/Model/Slot.h>

namespace ConversationEditor
{
    inline constexpr auto StringDataTypeName = "string";
    inline constexpr auto ActorTextDataTypeName = "actor_text";
    inline constexpr auto CommentDataTypeName = "comment";

    inline constexpr auto DialogueNodeActorTextSlotName = "inActorText";
    inline constexpr auto DialogueNodeDialogueIdSlotName = "inDialogueId";
    inline constexpr auto DialogueNodeIsStarterSlotName = "inIsStarter";
    inline constexpr auto DialogueNodeParentSlotName = "inParent";

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

    namespace CommonSlotDisplayNames
    {
        inline constexpr auto AVAILABILITYSCRIPT = "ActiveScript";
        inline constexpr const char* ActorText = "Text";
        inline constexpr const char* DialogueId = "Id";
        static const char* DIALOGUEDATA = "DialogueData";
        static const char* DIALOGUESCRIPT = "DialogueScriptId";
        static const char* ENTER = "Enter";
        static const char* EXIT = "Exit";
        inline constexpr const char* OutputDialogueId = "DialogueId";
        inline constexpr const char* ResponseTo = "Response To";
    } // namespace CommonSlotDisplayNames

    namespace CommonSlotNames
    {
        inline constexpr const char* AVAILABILITYSCRIPT = "AvailabilityScript";
        inline constexpr const char* DIALOGUEDATA = "DialogueData";
        inline constexpr const char* DIALOGUESCRIPT = "DialogueScript";
        inline constexpr const char* ENTER = "Enter";
        inline constexpr const char* EXIT = "Exit";
        inline constexpr const char* OutputDialogueId = "OutputDialogueId";
        inline constexpr const char* ResponseTo = "ResponseTo";
    } // namespace CommonSlotNames

    namespace CommonSlotDescriptions
    {
        inline constexpr const char* ActorText = "The text that will show up on screen.";
        inline constexpr const char* DialogueId = "The globally unique identifier for this specific dialogue.";
        static const char* DIALOGUEDATA = "Stores data related to a specific dialogue node.";
        static const char* DIALOGUESCRIPT = "An identifier for a script that runs when the dialogue is used.";
        static const char* ENTER = "";
        static const char* EXIT = "";
        inline constexpr const char* OutputDialogueId = DialogueId;
        inline constexpr const char* ResponseTo = "The node that this is a response to.";
    } // namespace CommonSlotDescriptions

    namespace NodeTypes
    {
        enum class DocumentEditorDataTypesEnum : GraphModel::DataType::Enum
        {
            InvalidEntity = 0, // Need a special case data type for the AZ::EntityId::InvalidEntityId to handle the default value since we
                               // are re-using the AZ::EntityId type in several node data types

            Dialogue,
            StringSetContainer
        };

        static const AZ::Uuid InvalidEntityTypeId{ "{3BB6E8E9-5525-4B88-A406-DBDE20E9FA5E}" };
    } // namespace NodeTypes

} // namespace ConversationEditor

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(ConversationEditor::NodeAndSlotNames, "543AB94D-202A-4210-8987-9CBD8668BC76"); // NOLINT
}
