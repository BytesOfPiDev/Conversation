#include "Tools/Window/Nodes/RootNode.h"

#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "AzCore/std/smart_ptr/make_shared.h"
#include "GraphModel/Integration/Helpers.h"
#include "GraphModel/Model/Common.h"
#include "GraphModel/Model/DataType.h"
#include "GraphModel/Model/Slot.h"

#include "Conversation/DialogueData.h"
#include "Tools/DataTypes.h"

namespace ConversationEditor
{
    AZ_RTTI_NO_TYPE_INFO_IMPL(RootNode); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(RootNode, AZ::SystemAllocator, 0); // NOLINT
    AZ_TYPE_INFO_WITH_NAME_IMPL(RootNode, "RootNode", RootNodeTypeId); // NOLINT

    constexpr auto RootNodeTitle = "Root";
    constexpr auto RootNodeSubTitle = "Start here.";

    constexpr int MinimumGreetings = 1;
    constexpr int MaximumGreetings = 10;

    RootNode::RootNode(GraphModel::GraphPtr graph)
        : GraphModel::Node(AZStd::move(graph))
    {
        RegisterSlots();
        CreateSlotData();
    }

    void RootNode::RegisterSlots()
    {
        using namespace NodeAndSlotNamesNamespace;

        AZStd::shared_ptr<GraphModel::DataType> const dialogueIdDataType = GetGraphContext()->GetDataType(AZ_CRC_CE("dialogue_id"));
        auto const dialogueId = Conversation::UniqueId::CreateRandomId();

        auto dialogueIdProperty = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Input,
            GraphModel::SlotType::Property,
            ToString(NodeAndSlotNames::NodeIdProperty),
            CommonSlotDisplayNames::DialogueId,
            CommonSlotDescriptions::DialogueId,
            GraphModel::DataTypeList{ dialogueIdDataType },
            AZStd::any(dialogueId));

        auto outputs = AZStd::make_shared<GraphModel::SlotDefinition>(
            GraphModel::SlotDirection::Output,
            GraphModel::SlotType::Data,
            ToString(NodeAndSlotNames::ConversationRootOutput_GreetingSlot),
            "Root",
            "Connect to a dialogue node to use that dialogue as a greeting.",
            GraphModel::DataTypeList{ dialogueIdDataType },
            AZStd::any(dialogueId),
            MinimumGreetings,
            MaximumGreetings,
            "Add Root",
            "",
            AZStd::vector<AZStd::string>(),
            true,
            false);

        RegisterSlot(AZStd::move(outputs));
    }

    void RootNode::Reflect(AZ::ReflectContext* context)
    {
        auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);

        if (serializeContext)
        {
            serializeContext->Class<RootNode, GraphModel::Node>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RootNode>("RootNode", "Node that represents a conversation at the highest level.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "Root");
            }
        }
    }

    auto RootNode::GetSubTitle() const -> char const*
    {
        return RootNodeSubTitle;
    }
    auto RootNode::GetTitle() const -> char const*
    {
        return RootNodeTitle;
    }

} // namespace ConversationEditor
