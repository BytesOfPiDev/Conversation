#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetManager.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/Entity.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/std/algorithm.h"
#include "AzCore/std/ranges/ranges_algorithm.h"
#include "AzTest/AzTest.h"
#include "Components/ConversationAssetRefComponent.h"
#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData.h"
#include "Conversation/UniqueId.h"
#include "ConversationTestEnvironment.h"
#include "DialogueComponent.h"
#include "DialogueComponentTestBase.h"

namespace ConversationTest
{
    class DialogueComponentTests : public DialogueComponentTestBase
    {
    protected:
        void SetUp() override
        {
            using namespace Conversation;

            DialogueComponentTestBase::SetUp();

            m_dialogueEntity->Init();
        }

        void TearDown() override
        {
            if (m_dialogueEntity->GetState() == AZ::Entity::State::Active)
            {
                m_dialogueEntity->Deactivate();
            }

            DialogueComponentTestBase::TearDown();
        }

        Conversation::DialogueData const m_dlg1{
            Conversation::UniqueId::CreateNamedId("TestDialogue01")
        };
        Conversation::DialogueData const m_dlg2{
            Conversation::UniqueId::CreateNamedId("TestDialogue02")
        };
    };

    TEST(
        UniqueIdStaticFunctionTests,
        CreateRandomId_CalledMultipleTimes_EachIdIsUnique)
    {
        using namespace Conversation;

        constexpr auto numberOfIdsToCreate = 50;
        AZStd::array<UniqueId, numberOfIdsToCreate> idContainer{};

        AZStd::generate(
            idContainer.begin(),
            idContainer.end(),
            []() -> UniqueId
            {
                return UniqueId::CreateRandomId();
            });

        AZStd::sort(idContainer.begin(), idContainer.end());
        auto const last = AZStd::unique(idContainer.begin(), idContainer.end());
        auto const uniqueItems{ AZStd::distance(idContainer.begin(), last) };

        EXPECT_EQ(uniqueItems, numberOfIdsToCreate);
    }

    TEST(DialogueDataConstruction, DefaultObject_HasInvalidId)
    {
        using namespace Conversation;

        DialogueData const defaultDialogueData{};
        EXPECT_FALSE(defaultDialogueData.IsValid());
    }

    TEST(DialogueDataTests, HasInvalidId_InitializingId_IsAssignedValidId)
    {
        using namespace Conversation;

        DialogueData defaultDialogueData{};
        EXPECT_FALSE(defaultDialogueData.IsValid());
        DialogueData::InitId(defaultDialogueData);
        EXPECT_TRUE(defaultDialogueData.IsValid());
    }

    TEST(DialogueDataTests, HasValidId_InitializingId_IdRemainsUnchanged)
    {
        using namespace Conversation;

        DialogueData dialogueData{ UniqueId::CreateRandomId() };
        EXPECT_TRUE(dialogueData.IsValid());

        auto givenId{ dialogueData.GetId() };
        DialogueData::InitId(dialogueData);

        EXPECT_EQ(givenId, dialogueData.GetId());
    }

    TEST(DialogueDataTests, NotAtMaxResponses_AddResponse_AddsTheResponseId)
    {
        using namespace Conversation;

        DialogueData dialogueData{};

        auto const responseId{ UniqueId::CreateRandomId() };

        dialogueData.AddResponseId(responseId);
        EXPECT_EQ(dialogueData.CountResponseIds(), 1);
        EXPECT_EQ(dialogueData.GetResponseIds().front(), responseId);
    }

    TEST(
        DialogueDataTests,
        NotAtMaxResponses_AddResponses_RejectsAddsThatGoPastMaxResponses)
    {
        using namespace Conversation;

        DialogueData dialogueData{};

        AZStd::array<UniqueId, DialogueData::MaxResponses> const
            containerOfMaxResponseIds =
                []() -> decltype(containerOfMaxResponseIds)
        {
            auto container = decltype(containerOfMaxResponseIds){};

            AZStd::generate(
                container.begin(),
                container.end(),
                []() -> UniqueId
                {
                    return UniqueId::CreateRandomId();
                });

            return AZStd::move(container);
        }();

        dialogueData.AddResponses(containerOfMaxResponseIds);
        EXPECT_EQ(dialogueData.CountResponseIds(), DialogueData::MaxResponses);

        dialogueData.AddResponseId(UniqueId::CreateRandomId());
        EXPECT_EQ(dialogueData.CountResponseIds(), DialogueData::MaxResponses);

        dialogueData.AddResponses(AZStd::array{ UniqueId::CreateRandomId(),
                                                UniqueId::CreateRandomId(),
                                                UniqueId::CreateRandomId() });
        EXPECT_EQ(dialogueData.CountResponseIds(), DialogueData::MaxResponses);

        EXPECT_TRUE(AZStd::ranges::all_of(
            dialogueData.GetResponseIds(),
            [](auto const& id) -> bool
            {
                return id.IsValid();
            }));
    }

    TEST(ConversationAssetTests, Defaulted_AddInvalidStartingId_IsRejected)
    {
        Conversation::ConversationAsset asset{};
        EXPECT_EQ(asset.CountStartingIds(), 0);
        asset.AddStartingId(Conversation::UniqueId{});
        EXPECT_EQ(asset.CountStartingIds(), 0);
    }

    TEST(ConversationAssetTests, Defaulted_AddValidStartingId_IsAdded)
    {
        using namespace Conversation;

        Conversation::ConversationAsset asset{};
        asset.AddStartingId(UniqueId::CreateRandomId());

        EXPECT_EQ(asset.CountStartingIds(), 1);
        EXPECT_TRUE(asset.CopyStartingIds().front().IsValid());
    }

    TEST(ConversationAssetTests, Defaulted_AddInvalidDialogue_IsRejected)
    {
        using namespace Conversation;

        Conversation::ConversationAsset asset{};
        asset.AddDialogue(
            DialogueData{ Conversation::UniqueId::CreateInvalidId() });

        EXPECT_EQ(asset.CountDialogues(), 0);
    }

    TEST(
        ConversationAssetTests,
        DefaultConstructed_AddingValidDialogueIsAccepted)
    {
        using namespace Conversation;

        ConversationAsset asset{};

        DialogueData const dlg1{ UniqueId::CreateRandomId() };
        DialogueData const dlg2{ UniqueId::CreateRandomId() };

        asset.AddDialogue(dlg1);
        asset.AddDialogue(dlg2);

        EXPECT_EQ(asset.CountDialogues(), 2);
    }

    TEST_F(DialogueComponentTests, Fixture_SanityCheck)
    {
        ASSERT_NE(m_dialogueEntity, nullptr);
        // EXPECT_NE(m_dialogueEntity->FindComponent(TagComponentType),
        // nullptr);
        /*
              EXPECT_NE(
                  m_dialogueEntity->FindComponent(DialogueComponentType),
           nullptr); EXPECT_NE(
                  m_dialogueEntity->FindComponent(ConversationAssetRefComponentType),
                  nullptr);
              EXPECT_EQ(m_dialogueEntity->GetState(), AZ::Entity::State::Init);
        */
    }

    TEST_F(
        DialogueComponentTests,
        DISABLED_CheckAvailability_GivenValidDialogueWithNoChecks_ReturnsTrue)
    {
        using namespace Conversation;

        DialogueData someValidDialogue{ UniqueId::CreateRandomId() };

        auto const resultWhenCheckedWithDialogueData =
            [this, &someValidDialogue]() -> bool
        {
            bool result{};
            DialogueComponentRequestBus::EventResult(
                result,
                m_dialogueEntity->GetId(),
                &DialogueComponentRequests::CheckAvailability,
                someValidDialogue);
            return result;
        }();

        EXPECT_TRUE(resultWhenCheckedWithDialogueData);

        auto const resultWhenCheckedWithId = [this,
                                              &someValidDialogue]() -> bool
        {
            bool result{};
            DialogueComponentRequestBus::EventResult(
                result,
                m_dialogueEntity->GetId(),
                &DialogueComponentRequests::CheckAvailabilityById,
                someValidDialogue.GetId());
            return result;
        }();

        EXPECT_TRUE(resultWhenCheckedWithId);
    }

    TEST(
        ConversationAssetRefComponentTests,
        ParentEntityActivated_RTTI_SanityCheck_NoCrash)
    {
        using namespace Conversation;

        AZ::Entity entity{ AZ::Entity::MakeId() };
        auto* component{ entity.CreateComponent(
            ConversationAssetRefComponent::TYPEINFO_Uuid()) };

        entity.Init();
        entity.Activate();

        EXPECT_NE(component, nullptr);
        EXPECT_TRUE(azrtti_istypeof<ConversationAssetRefComponent>(component));
        EXPECT_NE(
            entity.FindComponent<ConversationAssetRefComponent>(), nullptr);
    }

    TEST(
        ConversationAssetRefComponentTests,
        ParentEntityNotActivated_SetConversationAssetWithBus_AssetIsSet)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();
        AZ::Entity entity{ AZ::Entity::MakeId() };
        entity.CreateComponent(ConversationAssetRefComponent::TYPEINFO_Uuid());

        entity.Init();

        bool successfullyAssignedAsset{};
        ConversationAssetRefComponentRequestBus::EventResult(
            successfullyAssignedAsset,
            entity.GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            asset);

        EXPECT_TRUE(successfullyAssignedAsset);
    }

    TEST(
        ConversationAssetRefComponentTests,
        ParentEntityActivated_SetConversationAssetWithBus_AssetNotSet)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();
        AZ::Entity entity{ AZ::Entity::MakeId() };
        entity.CreateComponent(ConversationAssetRefComponent::TYPEINFO_Uuid());

        entity.Init();
        entity.Activate();

        bool successfullyAssignedAsset{};
        ConversationAssetRefComponentRequestBus::EventResult(
            successfullyAssignedAsset,
            entity.GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            asset);

        EXPECT_FALSE(successfullyAssignedAsset);
    }

    TEST(
        ConversationAssetRefComponentTests,
        HasValidStartingData_TryStartConversation_Success)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        AZ::Entity entity{};
        entity.CreateComponent(TagComponentType);
        entity.CreateComponent(ConversationAssetRefComponentType);
        entity.CreateComponent(DialogueComponentType);

        entity.Init();

        ConversationAssetRefComponentRequestBus::Event(
            entity.GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            asset);

        AZ::Component* const dialogueComponent =
            entity.FindComponent(DialogueComponentType);

        Conversation::DialogueComponentConfig dialogueComponentConfig{};
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        entity.Activate();

        auto const tryToStartConversationResult = [&entity]() -> bool
        {
            AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };

            bool result{};
            DialogueComponentRequestBus::EventResult(
                result,
                entity.GetId(),
                &DialogueComponentRequests::TryToStartConversation,
                someOtherEntityId);
            return result;
        }();

        EXPECT_TRUE(tryToStartConversationResult);

        DialogueState const getCurrentStateResult = [&entity]() -> DialogueState
        {
            auto result{ DialogueState::Invalid };
            DialogueComponentRequestBus::EventResult(
                result,
                entity.GetId(),
                &DialogueComponentRequests::GetCurrentState);

            return result;
        }();

        EXPECT_EQ(getCurrentStateResult, Conversation::DialogueState::Active);
    }

    TEST_F(
        DialogueComponentTests,
        HasBadStartingData_TryStartConversation_ReturnsFalse)
    {
        using namespace Conversation;

        m_dialogueEntity->Activate();

        auto const tryToStartConversationResult = [this]() -> bool
        {
            bool result{};
            AZ::EntityId const someOtherEntityId{ AZ::Entity::MakeId() };
            DialogueComponentRequestBus::EventResult(
                result,
                m_dialogueEntity->GetId(),
                &DialogueComponentRequests::TryToStartConversation,
                someOtherEntityId);

            return result;
        }();

        EXPECT_FALSE(tryToStartConversationResult);
    }

    TEST_F(
        DialogueComponentTests,
        TryStartConversation_CallWithValidData_SucceedsWithValidDialogueState)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        bool successfullyAssignedAsset{};
        ConversationAssetRefComponentRequestBus::EventResult(
            successfullyAssignedAsset,
            m_dialogueEntity->GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            asset);

        EXPECT_TRUE(successfullyAssignedAsset);

        m_dialogueEntity->Activate();

        AZ::EntityId someOtherEntityId{ AZ::Entity::MakeId() };

        bool success{ false };
        DialogueComponentRequestBus::EventResult(
            success,
            m_dialogueEntity->GetId(),
            &DialogueComponentRequests::TryToStartConversation,
            someOtherEntityId);

        EXPECT_TRUE(success);

        DialogueState dialogueCurrentState{ DialogueState::Invalid };
        DialogueComponentRequestBus::EventResult(
            dialogueCurrentState,
            m_dialogueEntity->GetId(),
            &DialogueComponentRequests::GetCurrentState);
        EXPECT_EQ(dialogueCurrentState, Conversation::DialogueState::Active);
    }

    TEST_F(
        DialogueComponentTests,
        ActiveConversation_WithResponsesAvailable_SelectingOutOfBoundsResponseIsRejected)
    {
        using namespace Conversation;

        auto asset = CreateStartableAsset();

        ConversationAssetRefComponentRequestBus::Event(
            m_dialogueEntity->GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            asset);

        m_dialogueEntity->Activate();
        auto dialogueEntityId{ m_dialogueEntity->GetId() };

        DialogueComponentRequestBus::Event(
            dialogueEntityId,
            &DialogueComponentRequests::TryToStartConversation,
            AZ::Entity::MakeId());

        constexpr auto getActiveDialogue =
            [](AZ::EntityId const& entityId) -> AZ::Outcome<DialogueData>
        {
            AZ::Outcome<DialogueData> resultOutcome{};
            DialogueComponentRequestBus::EventResult(
                resultOutcome,
                entityId,
                &DialogueComponentRequests::GetActiveDialogue);
            return resultOutcome;
        };

        // Test upper bound
        {
            auto const activeDialogueOutcomeBeforeSelection =
                getActiveDialogue(dialogueEntityId);

            auto const numberOfAvailableResponses = [this]() -> size_t
            {
                AZStd::vector<DialogueData> availableResponses{};
                DialogueComponentRequestBus::EventResult(
                    availableResponses,
                    m_dialogueEntity->GetId(),
                    &DialogueComponentRequests::GetAvailableResponses);
                return availableResponses.size();
            }();

            DialogueComponentRequestBus::Event(
                m_dialogueEntity->GetId(),
                &DialogueComponentRequests::SelectAvailableResponse,
                numberOfAvailableResponses + 1);

            auto const activeDialogueOutcomeAfterSelection =
                getActiveDialogue(m_dialogueEntity->GetId());

            EXPECT_EQ(
                activeDialogueOutcomeBeforeSelection.GetValue(),
                activeDialogueOutcomeAfterSelection.GetValue());
        }

        // Test lower bound
        {
            auto const activeDialogueOutcomeBeforeSelection =
                getActiveDialogue(dialogueEntityId);

            DialogueComponentRequestBus::Event(
                m_dialogueEntity->GetId(),
                &DialogueComponentRequests::SelectAvailableResponse,
                FirstResponseNumber - 1);

            auto const activeDialogueOutcomeAfterSelection =
                getActiveDialogue(dialogueEntityId);

            EXPECT_EQ(
                activeDialogueOutcomeBeforeSelection.GetValue(),
                activeDialogueOutcomeAfterSelection.GetValue());
        }
    }

    TEST_F(
        DialogueComponentTests,
        TryStartConversation_CallWithInvalidData_FailsWithInactiveDialogueState)
    {
        AZ::Component* const dialogueComponent =
            m_dialogueEntity->FindComponent(
                AZ::TypeId{ Conversation::DialogueComponentTypeId });

        Conversation::DialogueComponentConfig dialogueComponentConfig{};
        dialogueComponent->SetConfiguration(dialogueComponentConfig);

        m_dialogueEntity->Activate();

        auto* dialogueComponentRequests =
            Conversation::DialogueComponentRequestBus::FindFirstHandler(
                m_dialogueEntity->GetId());
        EXPECT_NE(dialogueComponentRequests, nullptr);

        EXPECT_FALSE(dialogueComponentRequests->TryToStartConversation(
            AZ::Entity::MakeId()));
        EXPECT_EQ(
            dialogueComponentRequests->GetCurrentState(),
            Conversation::DialogueState::Inactive);
    }

    TEST_F(
        DialogueComponentTests,
        SelectDialogue_GivenValidDialogue_DialogueBecomesActive)
    {
        using namespace Conversation;

        EXPECT_TRUE(false);
    }

    TEST_F(
        DialogueComponentTests,
        SelectDialogue_GivenInvalidDialogue_DialogueDoesNotBecomeActive)
    {
        using namespace Conversation;

        EXPECT_TRUE(false);
    }

    TEST_F(
        DialogueComponentTests,
        TrySelectDialogue_GivenIdExists_MatchingDialogueBecomesActive)
    {
        EXPECT_TRUE(false);
    }

    TEST_F(
        DialogueComponentTests,
        TrySelectDialogue_GivenIdNotFound_ActiveDialogueUnchanged)
    {
        EXPECT_TRUE(false);
    }

    TEST_F(
        DialogueComponentTests,
        TrySelectDialogue_GivenIdIsInvalid_ActiveDiaogueUnchanged)
    {
        EXPECT_TRUE(false);
    }

    TEST(ConversationAssetRefComponentTests, Fixture_SanityCheck)
    {
        EXPECT_TRUE(AZ::Data::AssetManager::IsReady());
    }

    TEST(
        ConversationAssetRefComponentTests,
        Constucted_NotConnectedToAssetRefComponentBus)
    {
        Conversation::ConversationAssetRefComponent assetRefComponent{};
        EXPECT_FALSE(assetRefComponent.BusIsConnected());
    }

    TEST(
        ConversationAssetRefComponentTests,
        ComponentOnAlreadyInitializedEntity_AssignAssetWithBus_Success)
    {
        using namespace Conversation;

        AZ::Entity entity{ AZ::Entity::MakeId() };
        entity.CreateComponent(ConversationAssetRefComponentType);
        entity.Init();

        auto const randomAssetId{ AZ::Uuid::CreateRandom() };
        auto const originalAsset =
            AZ::Data::AssetManager::Instance()
                .CreateAsset<Conversation::ConversationAsset>(
                    randomAssetId, AZ::Data::AssetLoadBehavior::PreLoad);

        ConversationAssetRefComponentRequestBus::Event(
            entity.GetId(),
            &ConversationAssetRefComponentRequests::SetConversationAsset,
            originalAsset);

        auto const returnedAsset =
            [&entity]() -> AZ::Data::Asset<ConversationAsset>
        {
            AZ::Data::Asset<ConversationAsset> result{};
            ConversationAssetRefComponentRequestBus::EventResult(
                result,
                entity.GetId(),
                &ConversationAssetRefComponentRequests::GetConversationAsset);
            return result;
        }();

        EXPECT_EQ(returnedAsset, originalAsset)
            << "Expected the returned asset to be "
            << originalAsset.ToString<AZStd::string>().c_str()
            << ", but received "
            << returnedAsset.ToString<AZStd::string>().c_str();
    }

} // namespace ConversationTest

AZ_UNIT_TEST_HOOK(new ConversationTest::ConversationTestEnvironment) // NOLINT
