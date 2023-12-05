#pragma once

#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <GraphModel/Model/Node.h>

#include "Conversation/ConversationTypeIds.h"

namespace ConversationEditor
{
    namespace Nodes
    {
        class ActorDialogue : public GraphModel::Node
        {
        public:
            AZ_CLASS_ALLOCATOR(ActorDialogue, AZ::SystemAllocator, 0); // NOLINT
            AZ_RTTI(ActorDialogue, ActorDialogueNodeTypeId, GraphModel::Node); // NOLINT

            static void Reflect(AZ::ReflectContext* context);

            ActorDialogue() = default;
            ActorDialogue(const ActorDialogue&) = delete;
            ActorDialogue(ActorDialogue&&) = delete;
            auto operator=(const ActorDialogue&) -> ActorDialogue& = delete;
            auto operator=(ActorDialogue&&) -> ActorDialogue& = delete;
            ActorDialogue(GraphModel::GraphPtr graph);
            ~ActorDialogue() override = default;

            auto GetSubTitle() const -> const char* override
            {
                return "An actor's dialogue.";
            }

            auto GetTitle() const -> const char* override
            {
                return "Actor Dialogue";
            }

        protected:
            auto CanExtendSlot(GraphModel::SlotDefinitionPtr slotDefinition) const -> bool override;
            void RegisterSlots() override;

            void PostLoadSetup(GraphModel::GraphPtr graph, GraphModel::NodeId id) override;
            void PostLoadSetup() override;

        private:
            AZStd::any m_scriptCanvasConfigAsAny;
        };
    } // namespace Nodes
} // namespace ConversationEditor
