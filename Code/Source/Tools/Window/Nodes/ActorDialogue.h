#pragma once

#include "GraphModel/Model/Node.h"

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
            AZ_DISABLE_COPY_MOVE(ActorDialogue); // NOLINT

            static void Reflect(AZ::ReflectContext* context);

            ActorDialogue() = default;
            ActorDialogue(GraphModel::GraphPtr graph);
            ~ActorDialogue() override = default;

            auto GetSubTitle() const -> char const* override
            {
                return "An actor's dialogue.";
            }

            auto GetTitle() const -> char const* override
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
