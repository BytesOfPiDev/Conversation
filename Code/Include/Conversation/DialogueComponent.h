#pragma once

#include "AzCore/Component/Component.h"

#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    class ConversationAssetRefComponentRequests;

    /**
     * @brief Allows an entity to use the conversation system.
     *
     * @note It is not necessary to have any dialogue assets assigned to the
     * component. It provides the basic information about the entity, such as
     * name, speaker tag, etc.
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
        auto WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const -> bool override;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        /**
         * @brief Tries to start a conversation if possible.
         *
         * @return True if it was successfully started, false otherwise.
         */
        auto TryToStartConversation(const AZ::EntityId& initiatingEntityId) -> bool override;
        /**
         * Forces the conversation to end. Triggers abort scripts.
         *
         * Intended for situations where an event takes place that must cancel the conversation.
         */
        void AbortConversation() override;
        /**
         * Ends the conversation normally. Triggers end scripts.
         */
        void EndConversation() override;
        /**
         * Makes the given dialogue the active dialogue.
         *
         * The given dialogue does not have to be a part of the asset.
         *
         * @param dialogueToSelect The dialogue that will be made active.
         */
        void SelectDialogue(DialogueData const& dialogueToSelect) override;
        /**
         * Attempts to find a dialogue matching the given ID and makes it active.
         *
         * Does nothing if no matching ID is found.
         *
         * @param dialogueId The ID of a dialogue contained in an attached ConversationAssetRefComponent.
         */
        auto TryToSelectDialogue(DialogueId const dialogueId) -> bool override;
        /**
         * Processes and sends out the index matching an available dialogue choice.
         *
         * For example, if the active dialogue has 4 available responses, you can select the second
         * one by passing in an index of '1'. It's assumed these options are being presented on screen
         * in random order, so it's up to the caller match selections with an index.
         *
         */
        void SelectAvailableResponse(int const responseNumber) override;
        /**
         * \brief Attempts to move the conversation along by selecting the next dialogue.
         *
         * What this function does depends what responses are available for the currently
         * active dialogue.
         *
         * If no responses are available, it ends the conversation.
         *
         * If the first response available is an NPC response, it selects it.
         *
         * If the first response is a player response, it sends out *all* available player
         * responses found in the response list as choices.
         */
        void ContinueConversation() override;

        [[nodiscard]] auto GetActiveDialogue() const -> AZ::Outcome<DialogueData> override
        {
            return m_activeDialogue ? *m_activeDialogue : DialogueData();
        }

        [[nodiscard]] auto GetAvailableResponses() const -> AZStd::vector<DialogueData> override
        {
            return m_availableResponses;
        }

        [[nodiscard]] auto GetCurrentState() const -> DialogueState override
        {
            return m_currentState;
        }

        [[nodiscard]] auto CheckAvailability(DialogueData const& dialogueData) -> bool override;
        [[nodiscard]] auto CheckAvailability(DialogueId const& dialogueIdToCheck) -> bool override;

    private:
        ConversationAssetRefComponentRequests* m_conversationAssetRequests{};
        DialogueComponentConfig m_config;
        ConversationAsset m_memoryConversationAsset;
        DialogueState m_currentState = DialogueState::Inactive;
        AZStd::optional<DialogueData> m_activeDialogue;
        AZStd::vector<DialogueData> m_availableResponses;
    };

} // namespace Conversation
