#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/Entity.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData.h"
#include "Conversation/UniqueId.h"
#include <gtest/gtest.h>

namespace ConversationTest
{

    constexpr AZ::TypeId TestConversationAssetId{
        "{3EBCCFAF-4574-4C56-BDD3-09EDC9CB8405}"
    };

    constexpr AZ::TypeId TagComponentType{ AZ::TypeId{
        Conversation::TagComponentTypeId } };
    constexpr AZ::TypeId ConversationAssetRefComponentType{ AZ::TypeId{
        Conversation::ConversationAssetRefComponentTypeId } };
    constexpr AZ::TypeId DialogueComponentType{
        Conversation::DialogueComponentTypeId
    };

    auto CreateStartableAsset()
        -> AZ::Data::Asset<Conversation::ConversationAsset>
    {
        using namespace Conversation;

        auto startableAsset =
            AZ::Data::AssetManager::Instance()
                .FindOrCreateAsset<Conversation::ConversationAsset>(
                    TestConversationAssetId,
                    AZ::Data::AssetLoadBehavior::PreLoad);

        // Requires at least one dialogue with a valid DialogueId.
        DialogueData startingDialogue1{ UniqueId::CreateNamedId(
            "StartableAssetDialogueId1") };
        startingDialogue1.SetShortText("Hello, where are you from?");
        startableAsset->AddDialogue(startingDialogue1);

        // Requires at least one valid starting Id that matches a dialogue
        // inside the asset.
        startableAsset->AddStartingId(startingDialogue1.GetId());

        DialogueData earthResponseDialogue{ UniqueId::CreateRandomId() };
        earthResponseDialogue.SetShortText("I am from Earth, duh.");
        startableAsset->AddResponse(
            { startingDialogue1.GetId(), earthResponseDialogue.GetId() });

        return startableAsset;
    }

    class DialogueComponentTestBase : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            using namespace Conversation;

            m_dialogueEntity =
                AZStd::make_unique<AZ::Entity>("DialogueEntityBase");

            m_dialogueEntity->CreateComponent(TagComponentType);

            auto* const conversationAssetRefComponent =
                m_dialogueEntity->CreateComponent(
                    ConversationAssetRefComponentType);

            m_assetRefRequests =
                azrtti_cast<ConversationAssetRefComponentRequests*>(
                    conversationAssetRefComponent);

            m_dialogueEntity->CreateComponent(DialogueComponentType);
        }

        void TearDown() override
        {
            m_catalogRequests = nullptr;
            m_dialogueEntity = nullptr;
        }

        AZStd::unique_ptr<AZ::Entity> m_dialogueEntity{};
        AZ::Data::AssetCatalogRequests* m_catalogRequests{};
        Conversation::ConversationAssetRefComponentRequests*
            m_assetRefRequests{};
    };

} // namespace ConversationTest
