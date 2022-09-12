#pragma once

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/std/parallel/lock.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponentBus.h>
#include <LmbrCentral/Rendering/MaterialAsset.h>

namespace Conversation
{
    enum class ConversationStates
    {
        Inactive,
        Starting,
        Active,
        Aborting,
        Ending
    };

    /**
     * Represents a conversation.
     *
     * Currently, an entity can only use one instance of this component. However,
     * it would be useful to allow multiple instances in the future in order to allow
     * separation of different conversations that may be added to the entity later.
     *
     * The m_owner AZ::EntityId is the entity you want to "own" the dialogue. This
     * allows you to have the DialogueComponent on a different entity than the one
     * it's attached to - it will listen to requests and send notifications using
     * that AZ::EntityId.
     *
     * @author Andre
     */
    class DialogueComponent
        : public AZ::Component
        , public AZ::EntityBus::Handler
        , public DialogueComponentRequestBus::Handler
    {
    public:
        AZ_COMPONENT(DialogueComponent, "{C7AFDF51-ECCC-4BD3-8A56-0763ED87CB5B}");

        DialogueComponent() = default;
        ~DialogueComponent() override;

        static void Reflect(AZ::ReflectContext* context);

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        DialogueIdUnorderedSetContainer GetStartingIds() const override
        {
            return m_startingIds;
        }

        DialogueDataUnorderedSetContainer GetDialogues() const override
        {
            return m_dialogues;
        }

        DialogueData FindDialogue(const DialogueId& dialogueId) const override
        {
            auto foundIter = m_dialogues.find(DialogueData(dialogueId));
            // We return a default created object if we didn't find one with the given ID.
            // It's up to the caller to check that the ID is non-null to confirm that a
            // valid DialogueData was found.
            // @todo Implement either an overload or another function that shows failure, such
            // as a function that returns AZ::Outcome
            return foundIter != m_dialogues.end() ? *foundIter : DialogueData();
        }

        void TryToStartConversation(const AZ::EntityId& initiatingEntityId) override;
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
        void SelectDialogue(const DialogueData& dialogueToSelect) override;
        /**
         * Processes and sends out a dialogue matching the given DialogueId.
         *
         * Does nothing if no matching ID is found.
         *
         * @param dialogueId The ID of a dialogue contained in one of the assets attached to the parent entity.
         *
         */
        void SelectDialogue(const DialogueId dialogueId) override;
        /**
         * Processes and sends out the index matching an available dialogue choice.
         *
         * For example, if the active dialogue has 4 available responses, you can select the second
         * one by passing in an index of '1'. It's assumed these options are being presented on screen
         * in random order, so it's up to the caller match selections with an index.
         *
         */
        void SelectAvailableResponse(const int availableResponseIndex);
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

        AZStd::string GetDisplayName() const
        {
            return m_displayName;
        }

        DialogueData GetActiveDialogue() const override
        {
            return m_activeDialogue ? *m_activeDialogue : DialogueData();
        }

        AZStd::vector<DialogueData> GetAvailableResponses() const override
        {
            return m_availableResponses;
        }

        static bool VerifyAvailability(const DialogueData& dialogueData);

    private:
        ConversationAssetContainer m_conversationAssets;
        AZ::Data::Asset<ConversationAsset> m_memoryConversationAsset;
        AZStd::unordered_set<DialogueId> m_startingIds;
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
        ConversationStates m_currentState = ConversationStates::Inactive;
        AZStd::unique_ptr<DialogueData> m_activeDialogue;
        AZStd::vector<DialogueData> m_availableResponses;
    };

} // namespace Conversation
