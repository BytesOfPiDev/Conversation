#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Asset/AssetManagerBus.h"
#include "AzCore/Component/Component.h"
#include "AzCore/std/string/conversions.h"
#include "AzTest/AzTest.h"
#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData.h"
#include "Conversation/DialogueData_incl.h"
#include "ConversationTestEnvironment.h"

namespace ConversationTest
{
    constexpr AZ::Uuid TestConversationAssetId{ "3EBCCFAF-4574-4C56-BDD3-09EDC9CB8405" };
    static constexpr AZ::TypeId TagComponentType{ "{0F16A377-EAA0-47D2-8472-9EAAA680B169}" };
    static constexpr AZ::TypeId DialogueComponentType{ Conversation::DialogueComponentTypeId };

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
            DialogueData dialogue1{ DialogueId{ "StartableAssetDialogueId1" } };
            startableAsset->AddDialogue(dialogue1);
            // Requires at least one valid starting Id that matches a dialogue inside the asset.
            startableAsset->AddStartingId(GetDialogueId(dialogue1));

            return startableAsset;
        }

        AZStd::unique_ptr<AZ::Entity> m_dialogueEntity{};
        AZ::Data::AssetCatalogRequests* m_catalogRequests{};

        Conversation::DialogueData m_dlg1{ Conversation::DialogueId{ "TestDialogue01" } };
        Conversation::DialogueData m_dlg2{ Conversation::DialogueId{ "TestDialogue02" } };
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
        EXPECT_EQ(GetDialogueResponseIds(dialogueData).size(), 1);
        EXPECT_EQ(GetDialogueResponseIds(dialogueData).front(), responseId);
        for (auto index = 0; index < (DialogueData::MaxResponses * 2); ++index)
        {
            AddDialogueResponseId(dialogueData, DialogueId{ AZStd::string("TestId") + AZStd::to_string(index) });
        }

        EXPECT_EQ(GetDialogueResponseIds(dialogueData).size(), DialogueData::MaxResponses);
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

    TEST_F(ConversationAssetTests, DefaultConstructed_AddingInvalidResponseIsRejected)
    {
        Conversation::ConversationAsset asset{};
        Conversation::ResponseData const invalidResponseData{};
        asset.AddResponse(invalidResponseData);
        EXPECT_EQ(asset.GetResponses().size(), 0);
    }

    TEST_F(ConversationAssetTests, ConversationAsset_DefaultConstructed_AddingValidResponseIsAccepted)
    {
        using namespace Conversation;

        Conversation::ConversationAsset asset{};
        Conversation::ResponseData const validResponseData{ CreateRandomDialogueId(), CreateRandomDialogueId() };
        asset.AddResponse(validResponseData);
        EXPECT_EQ(asset.GetResponses().size(), 1);
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
        dialogueComponentConfig.m_assets.push_back(asset);

        AZ::Component* const dialogueComponent = m_dialogueEntity->FindComponent(DialogueComponentType);
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        m_dialogueEntity->Activate();

        AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };

        AZStd::optional<AZStd::string> errorMessage{};
        DialogueComponentRequestBus::EventResult(
            errorMessage, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

        DialogueState dialogueCurrentState{ DialogueState::Invalid };
        DialogueComponentRequestBus::EventResult(
            dialogueCurrentState, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetCurrentState);

        EXPECT_EQ(dialogueCurrentState, Conversation::DialogueState::Active) << errorMessage->c_str();
        EXPECT_FALSE(errorMessage.has_value());
    }

    TEST_F(DialogueComponentTests, TryStartConversation_OnFailure_ReturnsError)
    {
        using namespace Conversation;

        m_dialogueEntity->Activate();

        AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };

        AZStd::optional<AZStd::string> errorMessage{};
        DialogueComponentRequestBus::EventResult(
            errorMessage, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

        EXPECT_TRUE(errorMessage.has_value());
    }

    TEST_F(DialogueComponentTests, TryStartConversation_CallWithValidData_SucceedsWithValidDialogueState)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        Conversation::DialogueComponentConfig dialogueComponentConfig{};
        dialogueComponentConfig.m_assets.push_back(asset);

        AZ::Component* const dialogueComponent = m_dialogueEntity->FindComponent(AZ::TypeId{ Conversation::DialogueComponentTypeId });
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        m_dialogueEntity->Activate();

        AZ::EntityId someOtherEntityId{ AZ::Entity::MakeId() };

        AZStd::optional<AZStd::string> errorMessage{};
        DialogueComponentRequestBus::EventResult(
            errorMessage, m_dialogueEntity->GetId(), &DialogueComponentRequests::TryToStartConversation, someOtherEntityId);

        DialogueState dialogueCurrentState{ DialogueState::Invalid };
        DialogueComponentRequestBus::EventResult(
            dialogueCurrentState, m_dialogueEntity->GetId(), &DialogueComponentRequests::GetCurrentState);
        EXPECT_EQ(dialogueCurrentState, Conversation::DialogueState::Active) << errorMessage->c_str();
    }

    TEST_F(DialogueComponentTests, TryStartConversation_CallWithInvalidData_FailsWithInvalidDialogueState)
    {
        AZ::Component* const dialogueComponent = m_dialogueEntity->FindComponent(AZ::TypeId{ Conversation::DialogueComponentTypeId });

        Conversation::DialogueComponentConfig dialogueComponentConfig{};
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        m_dialogueEntity->Activate();

        auto* dialogueComponentRequests = Conversation::DialogueComponentRequestBus::FindFirstHandler(m_dialogueEntity->GetId());
        EXPECT_NE(dialogueComponentRequests, nullptr);

        auto const tryToStartConversationErrorMessage = dialogueComponentRequests->TryToStartConversation(AZ::Entity::MakeId());
        EXPECT_TRUE(tryToStartConversationErrorMessage.has_value());
        EXPECT_EQ(dialogueComponentRequests->GetCurrentState(), Conversation::DialogueState::Inactive);
    }

} // namespace ConversationTest

AZ_UNIT_TEST_HOOK(new ConversationTest::ConversationTestEnvironment) // NOLINT
