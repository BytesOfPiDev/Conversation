#pragma once

#include "AzCore/Component/Component.h"
#include "AzCore/Component/ComponentBus.h"
#include "AzCore/RTTI/ReflectContext.h"

#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"

namespace Conversation
{

    class ConversationAssetRefComponent
        : public AZ::Component
        , public ConversationAssetRefComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(
            ConversationAssetRefComponent,
            ConversationAssetRefComponentTypeId,
            ConversationAssetRefComponentRequests); // NOLINT
        AZ_DISABLE_COPY_MOVE(ConversationAssetRefComponent); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        ConversationAssetRefComponent() = default;
        ~ConversationAssetRefComponent() override = default;

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);

        [[nodiscard]] auto CountStartingIds() const -> size_t override;

        [[nodiscard]] auto CountDialogues() const -> size_t override;

        [[nodiscard]] auto CopyStartingIds() const
            -> AZStd::vector<UniqueId> override;

        [[nodiscard]] auto CopyDialogues() const
            -> AZStd::unordered_set<DialogueData> override;

        void AddStartingId(UniqueId const& newStartingId) override;
        void AddDialogue(DialogueData const& newDialogueData) override;
        void AddResponse(ResponseData const& responseData) override;

        [[nodiscard]] auto GetDialogueById(UniqueId const& dialogueId)
            -> AZ::Outcome<DialogueData> override;
        [[nodiscard]] auto CheckDialogueExists(UniqueId const& dialogueId)
            -> bool override;
        [[nodiscard]] auto GetMainScriptAsset() const
            -> AZ::Data::Asset<AZ::ScriptAsset> override;

        void AddChunk(DialogueChunk const& dialogueChunk) override;

        [[nodiscard]] auto GetConversationAsset() const
            -> AZ::Data::Asset<ConversationAsset> override;

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        auto SetConversationAsset(
            AZ::Data::Asset<ConversationAsset> replacementAsset)
            -> bool override;

    private:
        AZ::Data::Asset<ConversationAsset> m_asset{};
    };
} // namespace Conversation
