#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/std/parallel/lock.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponentBus.h>

namespace Conversation
{
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
            auto foundIter = m_dialogues.find(dialogueId);
            // We return a default createed object if we didn't find one with the given ID.
            // It's up to the caller to check that the ID is non-null to confirm that a
            // valid DialogueData was found.
            // @todo Implement either an overload or another function that shows failure, such
            // as a function that returns AZ::Outcome
            return foundIter != m_dialogues.end() ? *foundIter : DialogueData();
        }

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
         * 
         * \note Currently, this is not considered, nor should it be, the same as a character's
         * ID. The reason is that this speaker tag is added and removed during activation/deactivate.
         * We don't want a character's ID being removed accidentally. Character ID's are not yet
         * implemented, so this will be looked at down the line.
         */
        AZStd::string m_speakerTag;
    };

} // namespace Conversation
