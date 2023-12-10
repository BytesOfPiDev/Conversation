#include "ConversationAssetRefComponent.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/Constants.h"

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
    }

    void ConversationAssetRefComponent::Activate()
    {
    }

    void ConversationAssetRefComponent::Deactivate()
    {
    }

    void ConversationAssetRefComponent::GetProvidedServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationAssetRefService"));
    }

    void ConversationAssetRefComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void ConversationAssetRefComponent::GetRequiredServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("DialogueService"));
    }

    void ConversationAssetRefComponent::GetDependentServices [[maybe_unused]] (AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    auto ConversationAssetRefComponent::CountStartingIds() const -> size_t
    {
        return m_asset->CountStartingIds();
    }

    auto ConversationAssetRefComponent::CountDialogues() const -> size_t
    {
        return m_asset->CountDialogues();
    }

    auto ConversationAssetRefComponent::GetStartingIds() const -> AZStd::vector<DialogueId> const&
    {
        return m_asset->GetStartingIds();
    }

    auto ConversationAssetRefComponent::CopyStartingIds() const -> AZStd::vector<DialogueId>
    {
        return m_asset->GetStartingIds();
    }

    auto ConversationAssetRefComponent::GetDialogues() const -> DialogueDataContainer const&
    {
        return m_asset->GetDialogues();
    }

    auto ConversationAssetRefComponent::CopyDialogues() const -> DialogueDataContainer
    {
        return m_asset->GetDialogues();
    }

    void ConversationAssetRefComponent::AddStartingId(DialogueId const& newStartingId)
    {
        return m_asset->AddStartingId(newStartingId);
    }

    void ConversationAssetRefComponent::AddDialogue(DialogueData const& newDialogueData)
    {
        m_asset->AddDialogue(newDialogueData);
    }

    void ConversationAssetRefComponent::AddResponse(ResponseData const& responseData)
    {
        m_asset->AddResponse(responseData);
    }

    auto ConversationAssetRefComponent::GetDialogueById(DialogueId const& dialogueId) -> AZ::Outcome<DialogueData>
    {
        return m_asset->GetDialogueById(dialogueId);
    }
    auto ConversationAssetRefComponent::CheckDialogueExists(DialogueId const& dialogueId) -> bool
    {
        return m_asset->CheckDialogueExists(dialogueId);
    }

    auto ConversationAssetRefComponent::GetMainScriptAsset() const -> AZ::Data::Asset<AZ::ScriptAsset>
    {
        return m_asset->GetMainScriptAsset();
    }

    void ConversationAssetRefComponent::AddChunk(DialogueChunk const& dialogueChunk)
    {
        m_asset->AddChunk(dialogueChunk);
    }
} // namespace Conversation
