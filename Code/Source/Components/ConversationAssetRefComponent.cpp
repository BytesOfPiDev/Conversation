#include "ConversationAssetRefComponent.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Component/Entity.h"
#include "AzCore/Outcome/Outcome.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/Constants.h"
#include "Conversation/DialogueData.h"
#include "Logging.h"

namespace Conversation
{
    void ConversationAssetRefComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationAssetRefComponentRequests>()->Version(1);
            serialize->Class<ConversationAssetRefComponent, AZ::Component, ConversationAssetRefComponentRequests>()->Version(0)->Field(
                "Asset", &ConversationAssetRefComponent::m_asset);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<ConversationAssetRefComponent>("Conversation Asset Ref", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Category, DialogueSystemCategory)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ConversationAssetRefComponent::m_asset, "Asset", "");
            }
        }
    }

    void ConversationAssetRefComponent::Init()
    {
        AZ_Assert( // NOLINT
            GetEntity() != nullptr,
            "Init should not be called if we're not connected to an entity!");

        ConversationAssetRefComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void ConversationAssetRefComponent::Activate()
    {
        AZ_Assert( // NOLINT
            GetEntity() != nullptr,
            "Activate should not be called if we're not connected to an entity!");
    }

    void ConversationAssetRefComponent::Deactivate()
    {
        AZ_Assert( // NOLINT
            GetEntity() != nullptr,
            "Deactivate should not be called if we're not connected to an entity!");
    }

    auto ConversationAssetRefComponent::GetConversationAsset() const -> AZ::Data::Asset<ConversationAsset>
    {
        return m_asset;
    }

    auto ConversationAssetRefComponent::SetConversationAsset(AZ::Data::Asset<ConversationAsset> replacementAsset) -> bool
    {
        if (GetEntity()->GetState() == AZ::Entity::State::Active)
        {
            LOG_EntityComponent(
                "LOG_ConversationAssetRefComponent",
                *this,
                "Failed to set asset. The asset cannot be changed while our entity is in the active state.");

            return false;
        }

        m_asset = replacementAsset;

        return true;
    }

    void ConversationAssetRefComponent::GetProvidedServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationAssetRefService"));
    }

    void ConversationAssetRefComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        // Multiple asset references are allowed on a single entity, for now. As a result, we don't add our provided service to the
        // incompatible list.
    }

    void ConversationAssetRefComponent::GetRequiredServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ConversationAssetRefComponent::GetDependentServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    auto ConversationAssetRefComponent::CountStartingIds() const -> size_t
    {
        return m_asset ? m_asset->CountStartingIds() : 0;
    }

    auto ConversationAssetRefComponent::CountDialogues() const -> size_t
    {
        return m_asset ? m_asset->CountDialogues() : 0;
    }

    auto ConversationAssetRefComponent::CopyStartingIds() const -> AZStd::vector<UniqueId>
    {
        return m_asset ? m_asset->CopyStartingIds() : AZStd::vector<UniqueId>{};
    }

    auto ConversationAssetRefComponent::CopyDialogues() const -> DialogueDataContainer
    {
        return m_asset ? m_asset->CopyDialogues() : DialogueDataContainer{};
    }

    void ConversationAssetRefComponent::AddStartingId(UniqueId const& newStartingId)
    {
        m_asset ? m_asset->AddStartingId(newStartingId) : void();
    }

    void ConversationAssetRefComponent::AddDialogue(DialogueData const& newDialogueData)
    {
        m_asset ? m_asset->AddDialogue(newDialogueData) : void();
    }

    void ConversationAssetRefComponent::AddResponse(ResponseData const& responseData)
    {
        m_asset ? m_asset->AddResponse(responseData) : void();
    }

    auto ConversationAssetRefComponent::GetDialogueById(UniqueId const& dialogueId) -> AZ::Outcome<DialogueData>
    {
        return m_asset ? m_asset->GetDialogueById(dialogueId) : AZ::Failure();
    }
    auto ConversationAssetRefComponent::CheckDialogueExists(UniqueId const& dialogueId) -> bool
    {
        return m_asset ? m_asset->CheckDialogueExists(dialogueId) : false;
    }

    auto ConversationAssetRefComponent::GetMainScriptAsset() const -> AZ::Data::Asset<AZ::ScriptAsset>
    {
        return m_asset ? m_asset->GetMainScriptAsset() : AZ::Data::Asset<AZ::ScriptAsset>{};
    }

    void ConversationAssetRefComponent::AddChunk(DialogueChunk const& dialogueChunk)
    {
        m_asset ? m_asset->AddChunk(dialogueChunk) : void();
    }
} // namespace Conversation
