#include "..\Include\Conversation\DialogueComponent.h"
#include <Conversation/DialogueComponent.h>

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <Conversation/ConversationAsset.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>

namespace Conversation
{
    class BehaviorDialogueComponentNotificationBusHandler
        : public DialogueComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER_WITH_DOC(
            BehaviorDialogueComponentNotificationBusHandler,
            "{D9A79838-589F-4BE3-9D87-7CFE187A52D3}",
            AZ::SystemAllocator,
            OnDialogue,
            ({ "Dialogue", "The dialogue being that was selected/spoken." },
             { "AvailableResponses", "A container of dialogues you can choose as a response." }),
            OnConversationStarted,
            ({ "Initiator", "The entity that started this conversation; typically the player or the NPC that spoke to another NPC." }),
            OnConversationAborted,
            (),
            OnConversationEnded,
            (),
            OnResponseAvailable,
            ({ "AvailableDialogue", "An available response to the currently active dialogue." }));

        void OnDialogue(const DialogueData& dialogue, const AZStd::vector<DialogueData>& availableResponses)
        {
            Call(FN_OnDialogue, dialogue, availableResponses);
        }

        void OnConversationStarted(const AZ::EntityId initiatingEntityId) override
        {
            Call(FN_OnConversationStarted, initiatingEntityId);
        }

        void OnConversationEnded() override
        {
            Call(FN_OnConversationEnded);
        }

        void OnConversationAborted() override
        {
            Call(FN_OnConversationAborted);
        }

        void OnResponseAvailable(const DialogueData& availableDialogue)
        {
            Call(FN_OnResponseAvailable, availableDialogue);
        }
    };

    class BehaviorGlobalConversationNotificationBusHandler
        : public GlobalConversationNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER_WITH_DOC(
            BehaviorGlobalConversationNotificationBusHandler,
            "{BC042832-201F-4797-B97E-CC5165D60361}",
            AZ::SystemAllocator,
            OnConversationStarted,
            ({ "Initiator", "The entity wanting to speak to the target." }, { "Target", "The entity being spoken to." }),
            OnConversationAborted,
            ({ "Target", "The entity the conversation abort was called on." }),
            OnConversationEnded,
            ({ "Target", "The entity whose conversation ended (has the dialogue component that was used.)" }));

        void OnConversationStarted(AZ::EntityId initiator, AZ::EntityId target) override
        {
            Call(FN_OnConversationStarted, initiator, target);
        }

        void OnConversationAborted(AZ::EntityId target) override
        {
            Call(FN_OnConversationAborted, target);
        }

        void OnConversationEnded(AZ::EntityId target) override
        {
            Call(FN_OnConversationEnded, target);
        }
    };

    DialogueComponent::~DialogueComponent()
    {
    }

    void DialogueComponent::Reflect(AZ::ReflectContext* context)
    {
        ConversationAsset::Reflect(context);

        [[maybe_unused]] constexpr static const char* DIALOGUE_SYSTEM_CATEGORY = "Dialogue System";
        constexpr static const char* DIALOGUE_COMPONENT_CATEGORY = "Dialogue System/DialogueComponent";

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<DialogueComponent, AZ::Component>()
                ->Version(0)
                ->Field("Assets", &DialogueComponent::m_conversationAssets)
                ->Field("MemoryAsset", &DialogueComponent::m_memoryConversationAsset)
                ->Field("Dialogues", &DialogueComponent::m_dialogues)
                ->Field("StartingIds", &DialogueComponent::m_startingIds)
                ->Field("SpeakerTag", &DialogueComponent::m_speakerTag);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueComponent>("DialogueComponent", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueComponent::m_conversationAssets, "Assets",
                        "The conversation file that will be used.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueComponent::m_speakerTag, "Speaker Tag",
                        "Identifies this entity as the owner of any dialogue containing this speaker tag.");

                editContext->Class<AZStd::vector<AZ::Crc32>>("Crc Vector", "")->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true);
            }

            serializeContext->RegisterGenericType<AZStd::vector<AZ::Crc32>>();
        }

        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<DialogueComponentRequestBus>("DialogueComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Event("FindDialogueById", &DialogueComponentRequestBus::Events::FindDialogue)
                ->Event("GetDialogues", &DialogueComponentRequestBus::Events::GetDialogues)
                ->Event("GetStartingIds", &DialogueComponentRequestBus::Events::GetStartingIds)
                ->Event(
                    "TryToStartConversation", &DialogueComponentRequestBus::Events::TryToStartConversation,
                    { { { "Initiator", "The entity starting the conversation." } } })
                ->Event("FindDialogueById", &DialogueComponentRequestBus::Events::FindDialogue, { { { "DialogueId", "" } } })
                ->Event(
                    "DoesDialogueWithIdExist", &DialogueComponentRequestBus::Events::CheckIfDialogueIdExists, { { { "DialogueId", "" } } })
                ->Event("GetConversationAssets", &DialogueComponentRequestBus::Events::GetConversationAssets)
                ->Event("GetSpeakerTag", &DialogueComponentRequestBus::Events::GetSpeakerTag)
                // Specifying the specific overloaded SelectDialogue function that takes DialogueData as a parameter.
                ->Event<void (DialogueComponentRequestBus::Events::*)(const DialogueData&)>(
                    "SelectDialogue", &DialogueComponentRequestBus::Events::SelectDialogue,
                    { { { "DialogueData", "The dialogue to make active." } } })
                // Specifying the specific overloaded SelectDialogue function that takes DialogueId as a parameter.
                ->Event<void (DialogueComponentRequestBus::Events::*)(const DialogueId)>(
                    "SelectDialogueById", &DialogueComponentRequestBus::Events::SelectDialogue,
                    { { { "DialogueId", "The ID of the dialogue to look for and make active." } } })
                ->Event("ContinueConversation", &DialogueComponentRequestBus::Events::ContinueConversation);

            behaviorContext->EBus<DialogueComponentNotificationBus>("DialogueComponentNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Handler<BehaviorDialogueComponentNotificationBusHandler>();

            behaviorContext->EBus<GlobalConversationNotificationBus>("GlobalConversationNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Conversation")
                ->Handler<BehaviorGlobalConversationNotificationBusHandler>();
        }
    }

    void DialogueComponent::Init()
    {
        m_memoryConversationAsset.Create(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));
    }

    void DialogueComponent::Activate()
    {
        // Combine the starting IDs and dialogues into one container respectively.
        for (const AZ::Data::Asset<ConversationAsset> asset : m_conversationAssets)
        {
            if (!asset.IsReady())
            {
                return;
            }

            m_startingIds.insert(asset->GetStartingIds().begin(), asset->GetStartingIds().end());
            m_dialogues.insert(asset->GetDialogues().begin(), asset->GetDialogues().end());
        }

        // The TagComponent is used to communicate with speakers, so we add it as a tag upon activation.
        // It will need to be removed upon deactivation.
        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequestBus::Events::AddTag, AZ::Crc32(m_speakerTag));

        AZ_Warning("ConversationModel", m_startingIds.size() != 0, "No starting IDs found. There should be at least one.");
        AZ_Warning("ConversationModel", m_dialogues.size() != 0, "No dialogues found. There should be one (multiple, really).");

        DialogueComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void DialogueComponent::Deactivate()
    {
        // Just in case there's a conversation, we abort on deactivation.
        AbortConversation();

        DialogueComponentRequestBus::Handler::BusDisconnect(GetEntityId());

        // We don't want anyone talking to us using our speaker tag in a deactivated state.
        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequestBus::Events::RemoveTag, AZ::Crc32(m_speakerTag));
    }

    void DialogueComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DialogueComponentService"));
    }

    void DialogueComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("DialogueComponentService"));
    }

    void DialogueComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TagService"));
    }

    void DialogueComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void DialogueComponent::TryToStartConversation(const AZ::EntityId& initiatingEntityId)
    {
        AZ_Printf("DialogueComponent", "Trying to start a conversation on Entity: %s.\n", this->GetNamedEntityId().GetName().data());

        AZ_Warning(
            "DialogueComponent", m_currentState == ConversationStates::Inactive,
            "An attempt was made to start a conversation on Entity: %s, but we're not in an 'Inactive' state.",
            this->GetNamedEntityId().GetName().data());

        if (m_currentState != ConversationStates::Inactive || m_startingIds.empty() || m_dialogues.empty())
        {
            return;
        }

        m_currentState = ConversationStates::Starting;

        // Until we have proper dialogue availability checking implemented, we pick the first ID.
        // It is safe to access the first starting id iter because we checked for a non-empty
        // container earlier.
        auto startingDialogueIter = m_dialogues.find(DialogueData(*m_startingIds.begin()));
        if (startingDialogueIter == m_dialogues.end() || !startingDialogueIter->IsValid())
        {
            m_currentState = ConversationStates::Inactive;
            return;
        }

        m_currentState = ConversationStates::Active;
        DialogueComponentNotificationBus::Event(
            GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationStarted, initiatingEntityId);
        GlobalConversationNotificationBus::Broadcast(
            &GlobalConversationNotificationBus::Events::OnConversationStarted, initiatingEntityId, GetEntityId());

        SelectDialogue(*startingDialogueIter);
    }

    void DialogueComponent::AbortConversation()
    {
        m_currentState = ConversationStates::Aborting;
        m_startingIds.clear();
        m_dialogues.clear();
        m_activeDialogue = nullptr;
        m_currentState = ConversationStates::Inactive;
        DialogueComponentNotificationBus::Event(GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationAborted);
    }

    void DialogueComponent::EndConversation()
    {
        m_currentState = ConversationStates::Ending;
        m_startingIds.clear();
        m_dialogues.clear();
        m_activeDialogue = nullptr;
        m_currentState = ConversationStates::Inactive;
        DialogueComponentNotificationBus::Event(GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationEnded);
    }

    void DialogueComponent::SelectDialogue(const DialogueData& dialogueToSelect)
    {
        AZ_Assert(dialogueToSelect.IsValid(), "A valid dialogue is needed in order to make a selection.");

        if (!dialogueToSelect.IsValid())
        {
            return;
        }

        m_activeDialogue = AZStd::make_unique<DialogueData>(dialogueToSelect);
        m_availableResponses.clear();
        for (const DialogueId& responseId : m_activeDialogue->GetResponseIds())
        {
            const DialogueData responseDialogue = FindDialogue(responseId);
            // Availability Checks are not yet implemented, so we're only checking that
            // the dialogue is valid for now.
            if (responseDialogue.IsValid())
            {
                m_availableResponses.push_back(responseDialogue);
            }
        }

        DialogueComponentNotificationBus::Event(
            GetEntityId(), &DialogueComponentNotificationBus::Events::OnDialogue, *m_activeDialogue, m_availableResponses);
    }

    void DialogueComponent::SelectDialogue(const DialogueId dialogueId)
    {
        auto dialogueIter = m_dialogues.find(DialogueData(dialogueId));
        if (dialogueIter == m_dialogues.end())
        {
            return;
        }

        SelectDialogue(*dialogueIter);
    }

    void DialogueComponent::ContinueConversation()
    {
        AZ_Assert(m_activeDialogue, "Active dialogue is null. A valid active dialogue pointer is required to continue the conversation.");

        if (!m_activeDialogue)
        {
            return;
        }

        auto firstResponseIdIter = m_activeDialogue->GetResponseIds().begin();
        auto firstResponseDialogueIter = m_dialogues.find(DialogueData(*firstResponseIdIter));
        AZ_Assert(firstResponseDialogueIter != m_dialogues.end(), "The given ID was not found in the dialogue container.");

        SelectDialogue(*firstResponseDialogueIter);
    }

} // namespace Conversation
