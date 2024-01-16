#pragma once

#include "AzCore/Component/Component.h"

#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    class ConversationAssetRefComponentRequests;

    // When given a list of responses to a dialogue, what number do we associate
    // with the first response?
    constexpr auto FirstResponseNumber = 1;

    /**
     * @brief Allows an entity to use the conversation system.
     *
     * A DialogueComponentConfig can be used to specify settings
     * that describe the entity.
     */
    class DialogueComponent
        : public AZ::Component
        , public DialogueComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(DialogueComponent, DialogueComponentTypeId); // NOLINT
        AZ_DISABLE_COPY_MOVE(DialogueComponent); // NOLINT

        DialogueComponent() = default;
        ~DialogueComponent() override = default;

        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        auto ReadInConfig(AZ::ComponentConfig const* config) -> bool override;
        auto WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const
            -> bool override;

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);

        auto TryToStartConversation(AZ::EntityId initiatingEntityId)
            -> bool override;
        void AbortConversation() override;
        void SelectDialogue(DialogueData dialogueToSelect) override;
        auto TryToSelectDialogue(UniqueId const dialogueId) -> bool override;
        void SelectAvailableResponse(int const responseNumber) override;
        void ContinueConversation() override;

        [[nodiscard]] auto GetActiveDialogue() const
            -> AZ::Outcome<DialogueData> override
        {
            return m_activeDialogue ? *m_activeDialogue : DialogueData();
        }

        [[nodiscard]] auto GetAvailableResponses() const
            -> AZStd::vector<DialogueData> override
        {
            return m_availableResponses;
        }

        [[nodiscard]] auto GetCurrentState() const -> DialogueState override
        {
            return m_currentState;
        }

        [[nodiscard]] auto CheckAvailability(
            DialogueData const& dialogueData) const -> bool override;
        [[nodiscard]] auto CheckAvailabilityById(
            UniqueId const& dialogueIdToCheck) const -> bool override;

    protected:
        /***********************************************************************
         * @brief Checks which of the active dialogue's responses are available
         *        and updates related data.
         **********************************************************************/
        void UpdateAvailableResponses();

        /***********************************************************************
         * @brief Executes the current conversation's companion script.
         **********************************************************************/
        void RunDialogueScript() const;
        void PlayDialogueAudio() const;
        void RunCinematic() const;

        /**
         * Ends the conversation normally. Triggers end scripts.
         */
        void EndConversation();

    private:
        ConversationAssetRefComponentRequests* m_conversationAssetRequests{};
        DialogueComponentConfig m_config;
        ConversationAsset m_memoryConversationAsset;
        DialogueState m_currentState = DialogueState::Inactive;
        // The currently active dialogue, if there is one.
        AZStd::optional<DialogueData> m_activeDialogue;
        // Available responses to the active dialogue
        AZStd::vector<DialogueData> m_availableResponses;
    };

} // namespace Conversation
