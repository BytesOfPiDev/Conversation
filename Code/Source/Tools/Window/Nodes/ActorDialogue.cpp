#include "Tools/DataTypes.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "Core/Core.h"
#include "GraphModel/Integration/Helpers.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/GraphContext.h"
#include "GraphModel/Model/Slot.h"

#include "Conversation/DialogueData.h"
#include "Tools/Window/Nodes/ActorDialogue.h"

namespace ConversationEditor
{
    void ActorDialogue::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext =
                azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ActorDialogue, GraphModel::Node>()->Version(
                0);

            if (AZ::EditContext* editContext =
                    serializeContext->GetEditContext())
            {
                editContext
                    ->Class<ActorDialogue>(
                        "ActorDialogue", "Represents an actor's response.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        GraphModelIntegration::Attributes::TitlePaletteOverride,
                        "ActorDialogue");
            }
        }
    }

    ActorDialogue::ActorDialogue(GraphModel::GraphPtr graph)
        : GraphModel::Node(AZStd::move(graph))
    {
        RegisterSlots();
        CreateSlotData();
    }
    auto ActorDialogue::CanExtendSlot(
        GraphModel::SlotDefinitionPtr slotDefinition) const -> bool
    {
        return GraphModel::Node::CanExtendSlot(slotDefinition);
    }
    void ActorDialogue::RegisterSlots()
    {
        GraphModel::DataTypePtr const stringDataType =
            GetGraphContext()->GetDataType(AZ_CRC_CE("string"));
        GraphModel::DataTypePtr const dialogueIdDataType =
            GetGraphContext()->GetDataType(AZ_CRC_CE("dialogue_id"));
        GraphModel::DataTypePtr const sourceHandleDataType =
            GetGraphContext()->GetDataType(AZ_CRC_CE("script_canvas_handle"));
        GraphModel::DataTypePtr const entityIdDataType =
            GetGraphContext()->GetDataType(AZ_CRC_CE("entity_id"));

        auto nodeNameProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Input,
            GraphModel::SlotType::Property,
            ToString(DialogueNodeSlots::inName),
            "Name",
            "",
            GraphModel::DataTypeList{ stringDataType },
            stringDataType->GetDefaultValue(),
            1,
            1,
            "",
            "",
            AZStd::vector<AZStd::string>{},
            false,
            false);

        auto parentIdInput = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Input,
            GraphModel::SlotType::Data,
            ToString(DialogueNodeSlots::inParent),
            "Parent",
            "",
            GraphModel::DataTypeList{ dialogueIdDataType },
            dialogueIdDataType->GetDefaultValue(),
            1,
            1);

        auto conditionIdInput = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Input,
            GraphModel::SlotType::Data,
            ToString(DialogueNodeSlots::inCondition),
            "Condition",
            "",
            GraphModel::DataTypeList{ dialogueIdDataType },
            dialogueIdDataType->GetDefaultValue(),
            0,
            3,
            "Add Condition",
            "",
            AZStd::vector<AZStd::string>(),
            true,
            false);

        auto speakerProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Input,
            GraphModel::SlotType::Property,
            ToString(DialogueNodeSlots::inSpeaker),
            "Speaker",
            "The speaker.",
            GraphModel::DataTypeList{ stringDataType },
            stringDataType->GetDefaultValue(),
            1,
            1,
            "",
            "",
            AZStd::vector<AZStd::string>{ "", "owner", "player" },
            false,
            false);

        auto speakerTextProperty =
            AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input,
                GraphModel::SlotType::Property,
                ToString(DialogueNodeSlots::inShortText),
                "Short Text",
                "",
                GraphModel::DataTypeList{ stringDataType },
                stringDataType->GetDefaultValue(),
                1,
                1,
                "",
                "",
                AZStd::vector<AZStd::string>(),
                false,
                false);

        auto scriptAssetProperty =
            AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input,
                GraphModel::SlotType::Property,
                "Script",
                "Script",
                "The script",
                GraphModel::DataTypeList{ sourceHandleDataType },
                sourceHandleDataType->GetDefaultValue(),
                0,
                0,
                "",
                "",
                AZStd::vector<AZStd::string>(),
                false,
                false);

        auto dialogueIdOutput = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Output,
            GraphModel::SlotType::Data,
            ToString(DialogueNodeSlots::outDialogue),
            "Dialogue",
            "",
            GraphModel::DataTypeList{ dialogueIdDataType },
            dialogueIdDataType->GetDefaultValue(),
            1,
            1,
            "");

        // Inputs
        RegisterSlot(parentIdInput);
        RegisterSlot(conditionIdInput);
        // Properties
        RegisterSlot(nodeNameProperty);
        RegisterSlot(speakerProperty);
        RegisterSlot(speakerTextProperty);
        RegisterSlot(scriptAssetProperty);
        // Outputs
        RegisterSlot(dialogueIdOutput);
    }

    void ActorDialogue::PostLoadSetup(
        GraphModel::GraphPtr graph, GraphModel::NodeId id)
    {
        GraphModel::Node::PostLoadSetup(graph, id);
    }

    void ActorDialogue::PostLoadSetup()
    {
        GraphModel::Node::PostLoadSetup();
    }
} // namespace ConversationEditor
