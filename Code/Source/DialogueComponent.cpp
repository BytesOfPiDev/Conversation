#include <Conversation/DialogueComponent.h>

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/ConversationAsset.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>

namespace Conversation
{
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
            behaviorContext->EBus<DialogueComponentRequestBus>("DialogueComponentRequests")
                ->Attribute(AZ::Script::Attributes::Category, DIALOGUE_COMPONENT_CATEGORY)
                ->Event("FindDialogueById", &DialogueComponentRequestBus::Events::FindDialogue)
                ->Event("GetDialogues", &DialogueComponentRequestBus::Events::GetDialogues)
                ->Event("GetStartingIds", &DialogueComponentRequestBus::Events::GetStartingIds);
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

} // namespace Conversation
