#pragma once

#include "Atom/RPI.Reflect/Image/StreamingImageAsset.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/EntityBus.h"

#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/IConversationAsset.h"

namespace Conversation
{
    /**
     * @brief Allows an entity to use the conversation system.
     *
     * @note It is not necessary to have any dialogue assets assigned to the
     * component.
     *
     * @note Any entity that wants to be part of a conversation will need
     * an instance of this component.
     */
    class DialogueComponent
        : public AZ::Component
        , public AZ::EntityBus::Handler
        , public DialogueComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(DialogueComponent); // NOLINT
        // AZ_COMPONENT(DialogueComponent, DialogueComponentTypeId); // NOLINT
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
         * @brief Returns a container of DialogueId's that can be used to start a conversation.
         * @return The container of DialogueId's.
         *
         * @see DialogueId.
         */
        [[nodiscard]] auto GetStartingIds() const -> AZStd::vector<DialogueId> const& override
        {
            return m_startingIds;
        }

        [[nodiscard]] auto GetDialogues() const -> DialogueDataContainer const& override
        {
            return m_dialogues;
        }

        [[nodiscard]] auto FindDialogue(DialogueId const& dialogueId) const -> DialogueData override
        {
            auto foundIter = m_dialogues.find(DialogueData(dialogueId));

            // We return a default created object if we didn't find one with the given ID.
            // It's up to the caller to check that the ID is non-null to confirm that a
            // valid DialogueData was found.
            // @todo Implement either an overload or another function that shows failure, such
            // as a function that returns AZ::Outcome
            return foundIter != m_dialogues.end() ? *foundIter : DialogueData();
        }

        [[nodiscard]] auto GetConversationAssets() const -> ConversationAssetContainer const& override
        {
            return m_config.m_assets;
        };

        /**
         * @brief Tries to start a conversation if possible.
         *
         * @return A string error message on failure, and nothing otherwise.
         */
        auto TryToStartConversation(const AZ::EntityId& initiatingEntityId) -> bool override;
        void AbortConversation() override;
        void EndConversation() override;
        /**
         * Processes and sends out a dialogue.
         *
         * Does nothing if the dialogue isn't valid (null ID). The dialogue does not have to exist as
         * part of a dialogue asset - it can be created in memory, but be careful not to try to jump
         * back to it if it isn't part of an attached asset.
         *
         * @param dialogueToSelect The dialogue that will be sent out.
         *
         */
        void SelectDialogue(DialogueData const& dialogueToSelect) override;
        /**
         * Processes and sends out a dialogue matching the given DialogueId.
         *
         * Does nothing if no matching ID is found.
         *
         * @param dialogueId The ID of a dialogue contained in one of the assets attached to the parent entity.
         *
         */
        void SelectDialogue(DialogueId const dialogueId) override;
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

        [[nodiscard]] auto GetDisplayName() const -> AZStd::string override
        {
            return m_displayName;
        }

        [[nodiscard]] auto GetActiveDialogue() const -> DialogueData override
        {
            return m_activeDialogue ? *m_activeDialogue : DialogueData();
        }

        [[nodiscard]] auto GetAvailableResponses() const -> AZStd::vector<DialogueData> override
        {
            return m_availableResponses;
        }

        [[nodiscard]] auto GetSpeakerTag() const -> AZStd::string override
        {
            return m_speakerTag;
        }

        [[nodiscard]] auto GetCurrentState() const -> DialogueState override
        {
            return m_currentState;
        }

        [[nodiscard]] auto CheckAvailability(DialogueData const& dialogueData) -> bool override;
        [[nodiscard]] auto CheckAvailability(DialogueId const& dialogueIdToCheck) -> bool override;
        [[nodiscard]] auto CheckIfDialogueIdExists(DialogueId const& /*dialogueId*/) const -> bool override;

    private:
        DialogueComponentConfig m_config;
        ConversationAsset m_memoryConversationAsset;
        AZStd::vector<DialogueId> m_startingIds;
        AZStd::vector<DialogueId> m_dialogueIds;
        AZStd::unordered_set<DialogueData> m_dialogues;
        /**
         * An entity's speaker tag.
         *
         * Each entity that is part of a conversation should have a speaker tag. This helps
         * associate an entity with a DialogueData that has this value as its speaker.
         */
        AZStd::string m_speakerTag;
        AZStd::string m_displayName;
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_speakerIconPath;
        DialogueState m_currentState = DialogueState::Inactive;
        AZStd::optional<DialogueData> m_activeDialogue;
        AZStd::vector<DialogueData> m_availableResponses;
    };

} // namespace Conversation
