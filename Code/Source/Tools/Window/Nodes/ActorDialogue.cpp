#include "Conversation/DialogueData.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/Slot.h"
#include <Core/Core.h>
#include <Tools/Window/Nodes/ActorDialogue.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <Editor/Framework/Configuration.h>

#include <Tools/DataTypes.h>

#include <GraphModel/Integration/Helpers.h>
#include <GraphModel/Model/GraphContext.h>

namespace ConversationEditor
{
    namespace Nodes
    {
        void ActorDialogue::Reflect(AZ::ReflectContext* context)
        {
            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<ActorDialogue, GraphModel::Node>()->Version(0);

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<ActorDialogue>("ActorDialogue", "Represents an actor's response.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "ActorDialogue");
                }
            }
        }

        ActorDialogue::ActorDialogue(GraphModel::GraphPtr graph)
            : GraphModel::Node(AZStd::move(graph))
        {
            RegisterSlots();
            CreateSlotData();
        }
        auto ActorDialogue::CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition) const -> bool
        {
            return GraphModel::Node::CanExtendSlot(slotDefinition);
        }
        void ActorDialogue::RegisterSlots()
        {
            GraphModel::DataTypePtr const stringDataType = GetGraphContext()->GetDataType(AZ_CRC_CE("string"));
            GraphModel::DataTypePtr const dialogueIdDataType = GetGraphContext()->GetDataType(AZ_CRC_CE("dialogue_id"));
            GraphModel::DataTypePtr const sourceHandleDataType = GetGraphContext()->GetDataType(AZ_CRC_CE("script_canvas_handle"));
            GraphModel::DataTypePtr const entityIdDataType = GetGraphContext()->GetDataType(AZ_CRC_CE("entity_id"));

            auto nodeNameProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, ToString(NodeAndSlotNames::NodeNameProperty), "Node Name",
                "Node Name Description", GraphModel::DataTypeList{ stringDataType }, stringDataType->GetDefaultValue(), 1, 1, "", "",
                AZStd::vector<AZStd::string>{}, false, false);

            auto parentIdInput = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Data, ToString(NodeAndSlotNames::ActorDialogueInput_ParentId),
                "Parent", "The dialogue owning this response .", GraphModel::DataTypeList{ dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(), 1, 1);

            auto conditionIdInput = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Data, ToString(NodeAndSlotNames::ActorDialogueInput_ConditionId),
                "Condition", "", GraphModel::DataTypeList{ dialogueIdDataType }, dialogueIdDataType->GetDefaultValue(), 0, 3,
                "Add Condition", "", AZStd::vector<AZStd::string>(), true, false);

            auto linkToInput = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Data, "Link", "Link To",
                "A non-owning dialogue to which this is a response to.", GraphModel::DataTypeList{ dialogueIdDataType },
                dialogueIdDataType->GetDefaultValue(), 1, 1);

            auto speakerProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, ToString(NodeAndSlotNames::ActorDialogueProperty_Speaker),
                "Speaker", "The speaker.", GraphModel::DataTypeList{ stringDataType }, stringDataType->GetDefaultValue(), 1, 1, "", "",
                AZStd::vector<AZStd::string>{ "", "owner", "player" }, false, false);

            auto nodeIdProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, ToString(NodeAndSlotNames::NodeIdProperty),
                CommonSlotDisplayNames::DialogueId, CommonSlotDescriptions::DialogueId, GraphModel::DataTypeList{ dialogueIdDataType },
                AZStd::any(Conversation::CreateRandomDialogueId()), 1, 1, "", "", AZStd::vector<AZStd::string>(), false, false);

            auto speakerTextProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, ToString(NodeAndSlotNames::ActorDialogueProperty_Text),
                CommonSlotDisplayNames::ActorText, CommonSlotDescriptions::ActorText, GraphModel::DataTypeList{ stringDataType },
                stringDataType->GetDefaultValue(), 1, 1, "", "", AZStd::vector<AZStd::string>(), false, false);

            auto scriptAssetProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, "Script", "Script", "The script",
                GraphModel::DataTypeList{ sourceHandleDataType }, sourceHandleDataType->GetDefaultValue(), 0, 0, "", "",
                AZStd::vector<AZStd::string>(), false, false);

            auto dialogueIdOutput = AZStd::make_shared<GraphModel::SlotDefinition>(
                GraphModel::SlotDirection::Output, GraphModel::SlotType::Data, ToString(NodeAndSlotNames::ActorDialogueOutput_Id),
                CommonSlotDisplayNames::OutputDialogueId, CommonSlotDescriptions::OutputDialogueId,
                GraphModel::DataTypeList{ dialogueIdDataType }, dialogueIdDataType->GetDefaultValue(), 1, 1, "");

            /*
                      auto exitEvent = AZStd::make_shared<GraphModel::SlotDefinition>(
                          GraphModel::SlotDirection::Output, GraphModel::SlotType::Event,
               ToString(NodeAndSlotNames::ActorDialogueEvent_Exit), "Exit");
            */

            // Events
            //         RegisterSlot(enterEvent);
            //        RegisterSlot(exitEvent);
            // Inputs
            RegisterSlot(parentIdInput);
            RegisterSlot(conditionIdInput);
            RegisterSlot(linkToInput);
            // Properties
            RegisterSlot(nodeNameProperty);
            RegisterSlot(nodeIdProperty);
            RegisterSlot(speakerProperty);
            RegisterSlot(speakerTextProperty);
            RegisterSlot(scriptAssetProperty);
            // Outputs
            RegisterSlot(dialogueIdOutput);
        }

        void ActorDialogue::PostLoadSetup(GraphModel::GraphPtr graph, GraphModel::NodeId id)
        {
            GraphModel::Node::PostLoadSetup(graph, id);
        }

        void ActorDialogue::PostLoadSetup()
        {
            GraphModel::Node::PostLoadSetup();
        }
    } // namespace Nodes
} // namespace ConversationEditor
