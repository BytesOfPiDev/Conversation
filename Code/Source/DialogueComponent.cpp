#include <Conversation/DialogueComponent.h>

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
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
            ());

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
            serializeContext->Class<AZStd::vector<AZ::Crc32>>()->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true);
        }

        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<DialogueComponentRequestBus>("DialogueComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Event("FindDialogueById", &DialogueComponentRequestBus::Events::FindDialogue)
                ->Event("GetDialogues", &DialogueComponentRequestBus::Events::GetDialogues)
                ->Event("GetStartingIds", &DialogueComponentRequestBus::Events::GetStartingIds);

            behaviorContext->EBus<DialogueComponentNotificationBus>("DialogueComponentNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Handler<BehaviorDialogueComponentNotificationBusHandler>();
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
        AZStd::string initiatingEntityName = {};
        AzFramework::GameEntityContextRequestBus::BroadcastResult(
            initiatingEntityName, &AzFramework::GameEntityContextRequestBus::Events::GetEntityName, initiatingEntityId);

        AZ_Printf("DialogueComponent", "Trying to start a conversation on Entity: %s.\n", initiatingEntityName.c_str());

        AZ_Warning(
            "DialogueComponent", m_currentState != ConversationStates::Inactive,
            "An attempt was made to start a conversation on Entity: %s, but we're not in an 'Inactive' state.",
            initiatingEntityName.c_str());

        if (m_currentState != ConversationStates::Inactive || m_startingIds.empty() || m_dialogues.empty())
        {
            return;
        }

        m_currentState = ConversationStates::Starting;

        // Until we have proper dialogue availability checking implemented, we pick the first ID.
        // It is safe to access the first starting id iter because we checked for a non-empty
        // container earlier.
        auto startingDialogueIter = m_dialogues.find(*m_startingIds.begin());

        if (startingDialogueIter == m_dialogues.end())
        {
            m_currentState = ConversationStates::Inactive;
            return;
        }

        m_activeDialogue = AZStd::make_unique<DialogueData>(*startingDialogueIter);
        m_currentState = ConversationStates::Active;

        // We just copy the responses for now; availability checks aren't implemented yet.
        AZStd::vector<DialogueData> availableResponses(
            m_activeDialogue->GetResponseIds().begin(), m_activeDialogue->GetResponseIds().end());

        DialogueComponentNotificationBus::Event(
            GetEntityId(), &DialogueComponentNotificationBus::Events::OnDialogue, *m_activeDialogue, availableResponses);
    }

    void DialogueComponent::AbortConversation()
    {
    }

    void DialogueComponent::EndConversation()
    {
    }

} // namespace Conversation
