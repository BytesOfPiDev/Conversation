#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Component/Component.h"
#include "AzCore/std/string/conversions.h"
#include "AzTest/AzTest.h"
#include "Components/ConversationAssetRefComponent.h"
#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponent.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData.h"
#include "Conversation/DialogueData_incl.h"
#include "ConversationTestEnvironment.h"
#include <gtest/gtest.h>

namespace ConversationTest
{
    constexpr AZ::TypeId TestConversationAssetId{ "{3EBCCFAF-4574-4C56-BDD3-09EDC9CB8405}" };
    constexpr AZ::TypeId TagComponentType{ AZ::TypeId{ Conversation::TagComponentTypeId } };
    constexpr AZ::TypeId ConversationAssetRefComponentType{ AZ::TypeId{ Conversation::ConversationAssetRefComponentTypeId } };
    constexpr AZ::TypeId DialogueComponentType{ Conversation::DialogueComponentTypeId };

    static Conversation::DialogueId const ValidDialogueId = Conversation::DialogueId{ "ValidDialogueId" };

    class DialogueDataTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
        }

        void TearDown() override
        {
        }
    };

    class ConversationAssetTests : public ::testing::Test
    {
    protected:
        Conversation::DialogueData m_dlg1{ Conversation::DialogueId{ "TestDialogue01" } };
        Conversation::DialogueData m_dlg2{ Conversation::DialogueId{ "TestDialogue02" } };
    };

    class DialogueComponentTests : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            using namespace Conversation;

            m_dialogueEntity = AZStd::make_unique<AZ::Entity>("DialogueEntity");
            m_dialogueEntity->Init();

            m_dialogueEntity->CreateComponent(TagComponentType);
            m_dialogueEntity->CreateComponent(ConversationAssetRefComponentType);
            m_dialogueEntity->CreateComponent(DialogueComponentType);
        }

        void TearDown() override
        {
            m_dialogueEntity = nullptr;
        }

        auto CreateStartableAsset() -> AZ::Data::Asset<Conversation::ConversationAsset>
        {
            using namespace Conversation;

            auto startableAsset = AZ::Data::AssetManager::Instance().FindOrCreateAsset<Conversation::ConversationAsset>(
                TestConversationAssetId, AZ::Data::AssetLoadBehavior::PreLoad);

            // Requires at least one dialogue with a valid DialogueId.
            DialogueData startingDialogue1{ DialogueId{ "StartableAssetDialogueId1" } };
            SetDialogueActorText(startingDialogue1, "Hello, where are you from?");
            startableAsset->AddDialogue(startingDialogue1);

            // Requires at least one valid starting Id that matches a dialogue inside the asset.
            startableAsset->AddStartingId(GetDialogueId(startingDialogue1));

            DialogueData earthResponseDialogue{ CreateRandomDialogueId() };
            earthResponseDialogue.m_actorText = "I am from Earth, duh.";
            startableAsset->AddResponse({ GetDialogueId(startingDialogue1), GetDialogueId(earthResponseDialogue) });

            return startableAsset;
        }

        AZStd::unique_ptr<AZ::Entity> m_dialogueEntity{};
        AZ::Data::AssetCatalogRequests* m_catalogRequests{};

        Conversation::DialogueData m_dlg1{ Conversation::DialogueId{ "TestDialogue01" } };
        Conversation::DialogueData m_dlg2{ Conversation::DialogueId{ "TestDialogue02" } };
    };

    class ConversationAssetRefComponentTests : public ::testing::Test
    {
    };

    TEST_F(DialogueDataTests, DefaultObject_HasNullId)
    {
        using namespace Conversation;

        DialogueData const defaultDialogueData{};

        EXPECT_FALSE(IsValid(defaultDialogueData));
    }

    TEST_F(DialogueDataTests, DefaultObject_IsValidReturnsFalse)
    {
        using namespace Conversation;

        DialogueData const defaultDialogueData{};
        EXPECT_FALSE(IsValid(defaultDialogueData));
    }

    TEST_F(DialogueDataTests, AddResponse_CorrectlyAddsResponseId)
    {
        using namespace Conversation;

        DialogueData dialogueData{};
        EXPECT_TRUE(GetDialogueResponseIds(dialogueData).empty());

        DialogueId const responseId{ DialogueId{ "TestId" } };
        EXPECT_TRUE(IsValid(responseId));

        AddDialogueResponseId(dialogueData, responseId);
        EXPECT_EQ(CountDialogueResponseIds(dialogueData), 1);
        EXPECT_EQ(GetDialogueResponseIds(dialogueData).front(), responseId);

        for (auto index = 0; index < (DialogueData::MaxResponses + 1); ++index)
        {
            AddDialogueResponseId(dialogueData, DialogueId{ AZStd::string("TestId") + AZStd::to_string(index) });
        }

        EXPECT_EQ(CountDialogueResponseIds(dialogueData), DialogueData::MaxResponses);
    }

    TEST_F(ConversationAssetTests, DefaultConstructed_AddingInvalidStartingIdIsRejected)
    {
        Conversation::ConversationAsset asset{};
        EXPECT_EQ(asset.CountStartingIds(), 0);
        asset.AddStartingId(Conversation::DialogueId{});
        EXPECT_EQ(asset.CountStartingIds(), 0);
    }

    TEST_F(ConversationAssetTests, DefaultConstructed_AddingValidStartingIdIsAccepted)
    {
        using namespace Conversation;

        Conversation::ConversationAsset asset{};
        EXPECT_EQ(asset.CountStartingIds(), 0);
        asset.AddStartingId(CreateRandomDialogueId());
        EXPECT_EQ(asset.CountStartingIds(), 1);
    }

    TEST_F(ConversationAssetTests, DefaultConstructed_AddingInvalidDialogueIsRejected)
    {
        Conversation::ConversationAsset asset{};
        Conversation::DialogueData const defaultDialogueData{};
        asset.AddDialogue(defaultDialogueData);
        EXPECT_EQ(asset.CountDialogues(), 0);
    }

    TEST_F(ConversationAssetTests, DefaultConstructed_AddingValidDialogueIsAccepted)
    {
        Conversation::ConversationAsset asset{};

        asset.AddDialogue(m_dlg1);
        EXPECT_EQ(asset.CountDialogues(), 1);

        asset.AddDialogue(m_dlg2);
        EXPECT_EQ(asset.CountDialogues(), 2);
    }

    TEST_F(DialogueComponentTests, Fixture_SanityCheck)
    {
        ASSERT_NE(m_dialogueEntity, nullptr);
        EXPECT_NE(m_dialogueEntity->FindComponent(TagComponentType), nullptr);
        EXPECT_NE(m_dialogueEntity->FindComponent(DialogueComponentType), nullptr);
        EXPECT_EQ(m_dialogueEntity->GetState(), AZ::Entity::State::Init);
    }

    TEST_F(DialogueComponentTests, TryStartConversation_OnSuccess_NoError)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        Conversation::DialogueComponentConfig dialogueComponentConfig{};

        bool successfullyAssignedAsset{};
        ConversationAssetRefComponentRequestBus::EventResult(
            successfullyAssignedAsset, m_dialogueEntity->GetId(), &ConversationAssetRefComponentRequests::SetConversationAsset, asset);

        EXPECT_TRUE(successfullyAssignedAsset);

        AZ::Component* const dialogueComponent = m_dialogueEntity->FindComponent(DialogueComponentType);
        dialogueComponent->SetConfiguration(dialogueComponentConfig);
        m_dialogueEntity->Activate();

        AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };

        bool success{ false };
        DialogueComponentRequestBus::EventResult(
            success, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

        DialogueState dialogueCurrentState{ DialogueState::Invalid };
        DialogueComponentRequestBus::EventResult(
            dialogueCurrentState, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetCurrentState);

        EXPECT_EQ(dialogueCurrentState, Conversation::DialogueState::Active);
        EXPECT_TRUE(success);
    }

    TEST_F(DialogueComponentTests, TryStartConversation_OnFailure_ReturnsFalse)
    {
        using namespace Conversation;

        m_dialogueEntity->Activate();

        auto const conversationStarted = [this]() -> bool
        {
            bool result{};
            AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };
            DialogueComponentRequestBus::EventResult(
                result, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

            return result;
        }();

        EXPECT_FALSE(conversationStarted);
    }

    TEST_F(DialogueComponentTests, TryStartConversation_CallWithValidData_SucceedsWithValidDialogueState)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        bool successfullyAssignedAsset{};
        ConversationAssetRefComponentRequestBus::EventResult(
            successfullyAssignedAsset, m_dialogueEntity->GetId(), &ConversationAssetRefComponentRequests::SetConversationAsset, asset);

        EXPECT_TRUE(successfullyAssignedAsset);

        m_dialogueEntity->Activate();

        AZ::EntityId someOtherEntityId{ AZ::Entity::MakeId() };

        bool success{ false };
        DialogueComponentRequestBus::EventResult(
            success, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

        EXPECT_TRUE(success);

        DialogueState dialogueCurrentState{ DialogueState::Invalid };
        DialogueComponentRequestBus::EventResult(
            dialogueCurrentState, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetCurrentState);
        EXPECT_EQ(dialogueCurrentState, Conversation::DialogueState::Active);
    }

    TEST_F(DialogueComponentTests, ActiveConversation_WithResponsesAvailable_SelectingOutOfBoundsResponseIsRejected)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        ConversationAssetRefComponentRequestBus::Event(
            m_dialogueEntity->GetId(), &ConversationAssetRefComponentRequests::SetConversationAsset, asset);

        m_dialogueEntity->Activate();

        DialogueComponentRequestBus::Event(
            m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, AZ::Entity::MakeId());

        // Test upper bound
        {
            auto const activeDialogueOutcomeBeforeSelection = [this]() -> AZ::Outcome<DialogueData>
            {
                AZ::Outcome<DialogueData> resultOutcome{};
                DialogueComponentRequestBus::EventResult(
                    resultOutcome, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetActiveDialogue);
                return resultOutcome;
            }();

            auto const numberOfAvailableResponses = [this]() -> size_t
            {
                AZStd::vector<DialogueData> availableResponses{};
                DialogueComponentRequestBus::EventResult(
                    availableResponses, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetAvailableResponses);
                return availableResponses.size();
            }();

            DialogueComponentRequestBus::Event(
                m_dialogueEntity->GetId(), &DialogueComponentRequests::SelectAvailableResponse, numberOfAvailableResponses + 1);

            auto const activeDialogueOutcomeAfterSelection = [this]() -> AZ::Outcome<DialogueData>
            {
                AZ::Outcome<DialogueData> resultOutcome{};
                DialogueComponentRequestBus::EventResult(
                    resultOutcome, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetActiveDialogue);
                return resultOutcome;
            }();

            EXPECT_EQ(activeDialogueOutcomeBeforeSelection.GetValue(), activeDialogueOutcomeAfterSelection.GetValue());
        }

        // Test lower bound
        {
            auto const activeDialogueOutcomeBeforeSelection = [this]() -> AZ::Outcome<DialogueData>
            {
                AZ::Outcome<DialogueData> resultOutcome{};
                DialogueComponentRequestBus::EventResult(
                    resultOutcome, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetActiveDialogue);
                return resultOutcome;
            }();

            DialogueComponentRequestBus::Event(
                m_dialogueEntity->GetId(), &DialogueComponentRequests::SelectAvailableResponse, FirstResponseNumber - 1);

            auto const activeDialogueOutcomeAfterSelection = [this]() -> AZ::Outcome<DialogueData>
            {
                AZ::Outcome<DialogueData> resultOutcome{};
                DialogueComponentRequestBus::EventResult(
                    resultOutcome, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetActiveDialogue);
                return resultOutcome;
            }();

            EXPECT_EQ(activeDialogueOutcomeBeforeSelection.GetValue(), activeDialogueOutcomeAfterSelection.GetValue());
        }
    }

    TEST_F(DialogueComponentTests, TryStartConversation_CallWithInvalidData_FailsWithInactiveDialogueState)
    {
        AZ::Component* const dialogueComponent = m_dialogueEntity->FindComponent(AZ::TypeId{ Conversation::DialogueComponentTypeId });

        Conversation::DialogueComponentConfig dialogueComponentConfig{};
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        m_dialogueEntity->Activate();

        auto* dialogueComponentRequests = Conversation::DialogueComponentRequestBus::FindFirstHandler(m_dialogueEntity->GetId());
        EXPECT_NE(dialogueComponentRequests, nullptr);

        EXPECT_FALSE(dialogueComponentRequests->TryToStartConversation(AZ::Entity::MakeId()));
        EXPECT_EQ(dialogueComponentRequests->GetCurrentState(), Conversation::DialogueState::Inactive);
    }

    TEST_F(ConversationAssetRefComponentTests, Fixture_SanityCheck)
    {
        EXPECT_TRUE(AZ::Data::AssetManager::IsReady());
    }

    TEST_F(ConversationAssetRefComponentTests, Constucted_NotConnectedToAssetRefComponentBus)
    {
        Conversation::ConversationAssetRefComponent assetRefComponent{};
        EXPECT_FALSE(assetRefComponent.BusIsConnected());
    }

    TEST_F(ConversationAssetRefComponentTests, ComponentOnAlreadyInitializedEntity_AssignAssetWithBus_Success)
    {
        using namespace Conversation;

        AZ::Entity entity{ AZ::Entity::MakeId() };
        entity.CreateComponent(ConversationAssetRefComponentType);
        entity.Init();

        auto const randomAssetId{ AZ::Uuid::CreateRandom() };
        auto const originalAsset = AZ::Data::AssetManager::Instance().CreateAsset<Conversation::ConversationAsset>(
            randomAssetId, AZ::Data::AssetLoadBehavior::PreLoad);

        ConversationAssetRefComponentRequestBus::Event(
            entity.GetId(), &ConversationAssetRefComponentRequests::SetConversationAsset, originalAsset);

        auto const returnedAsset = [&entity]() -> AZ::Data::Asset<ConversationAsset>
        {
            AZ::Data::Asset<ConversationAsset> result{};
            ConversationAssetRefComponentRequestBus::EventResult(
                result, entity.GetId(), &ConversationAssetRefComponentRequests::GetConversationAsset);
            return result;
        }();

        EXPECT_EQ(returnedAsset, originalAsset) << "Expected the returned asset to be " << originalAsset.ToString<AZStd::string>().c_str()
                                                << ", but received " << returnedAsset.ToString<AZStd::string>().c_str();
    }

} // namespace ConversationTest

AZ_UNIT_TEST_HOOK(new ConversationTest::ConversationTestEnvironment) // NOLINT
