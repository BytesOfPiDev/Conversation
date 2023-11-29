#include "GraphModel/Model/Slot.h"
#include <Tools/Window/Nodes/Link.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/string/string.h>
#include <Conversation/DialogueData.h>
#include <GraphModel/Integration/Helpers.h>
#include <Tools/DataTypes.h>

#include <utility>

namespace ConversationEditor
{
    namespace Nodes
    {
        Link::Link(GraphModel::GraphPtr graph)
            : GraphModel::Node(AZStd::move(graph))
        {
            RegisterSlots();
            CreateSlotData();
        }

        void Link::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<Link, GraphModel::Node>()->Version(0);

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<Link>("Link", "Note that represent branching off to a dialogue defined elsewhere.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(GraphModelIntegration::Attributes::TitlePaletteOverride, "Link");
                }
            }
        }

        void Nodes::Link::RegisterSlots()
        {
            GraphModel::DataTypePtr uniqueIdDataType = GetGraphContext()->GetDataType<AZ::Uuid>();

            auto enterSlotDefinition = AZStd::make_shared<GraphModel::SlotDefinition>(GraphModel::SlotDefinition(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Event, CommonSlotNames::ENTER, CommonSlotDisplayNames::ENTER,
                CommonSlotDescriptions::ENTER));

            auto dialogueIdSlotDefinition = AZStd::make_shared<GraphModel::SlotDefinition>(GraphModel::SlotDefinition(
                GraphModel::SlotDirection::Input, GraphModel::SlotType::Property, SlotNames::LINKEDID, "Dialogue Id",
                "The unique ID of an existing dialogue within this graph.", { GetGraphContext()->GetDataType<Conversation::DialogueId>() },
                AZStd::make_any<Conversation::DialogueId>(), 1, 1, "Add Link"));

            RegisterSlot(AZStd::move(enterSlotDefinition));
        }
    } // namespace Nodes
} // namespace ConversationEditor
