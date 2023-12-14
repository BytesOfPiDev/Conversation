#include "Conversation/DialogueComponent.h"

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Component/Component.h"
#include "AzCore/Component/Entity.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptContextAttributes.h"
#include "AzCore/Serialization/SerializeContext.h"
#include "Conversation/Components/ConversationAssetRefComponentBus.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueComponentBus.h"
#include "Conversation/DialogueData_incl.h"
#include "LmbrCentral/Scripting/TagComponentBus.h"
#include "cstdlib"

#include "Conversation/AvailabilityBus.h"
#include "Conversation/Components/DialogueComponentConfig.h"
#include "Conversation/Constants.h"
#include "Conversation/ConversationAsset.h"
#include "Conversation/DialogueScript.h"
#include "Logging.h"

namespace Conversation
{
    // An entity with this tag is an entity currently in the middle of a conversation.
    constexpr auto ActiveConversationTag{ AZ_CRC_CE("active_conversation") };
    // The entity with this tag is the entity the player is in a conversation with.
    constexpr auto PlayerConversationTag{ AZ_CRC_CE("player_conversation") };
    constexpr auto PlayerSpeakerTag{ "player" };

    // When given a list of responses to a dialogue, what number do we associate with the first response?
    constexpr auto FirstResponseNumber = 1;

    class BehaviorDialogueComponentNotificationBusHandler
        : public DialogueComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER_WITH_DOC( // NOLINT
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

        AZ_DISABLE_COPY_MOVE(BehaviorDialogueComponentNotificationBusHandler); // NOLINT
        ~BehaviorDialogueComponentNotificationBusHandler() override = default;

        void OnDialogue(DialogueData const& dialogue, AZStd::vector<DialogueData> const& availableResponses) override
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

        void OnResponseAvailable(DialogueData const& availableDialogue) override
        {
            Call(FN_OnResponseAvailable, availableDialogue);
        }
    };

    class BehaviorGlobalConversationNotificationBusHandler
        : public GlobalConversationNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER_WITH_DOC( // NOLINT
            BehaviorGlobalConversationNotificationBusHandler,
            "{BC042832-201F-4797-B97E-CC5165D60361}",
            AZ::SystemAllocator,
            OnConversationStarted,
            ({ "Initiator", "The entity wanting to speak to the target." }, { "Target", "The entity being spoken to." }),
            OnConversationAborted,
            ({ "Target", "The entity the conversation abort was called on." }),
            OnConversationEnded,
            ({ "Target", "The entity whose conversation ended (has the dialogue component that was used.)" }));

        AZ_DISABLE_COPY_MOVE(BehaviorGlobalConversationNotificationBusHandler); // NOLINT
        ~BehaviorGlobalConversationNotificationBusHandler() override = default;

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

    void DialogueComponent::Reflect(AZ::ReflectContext* context)
    {
        ConversationAsset::Reflect(context);
        DialogueComponentConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<DialogueComponent, AZ::Component>()
                ->Version(2)
                ->Field("Config", &DialogueComponent::m_config)
                ->Field("MemoryAsset", &DialogueComponent::m_memoryConversationAsset);

            serializeContext->RegisterGenericType<AZStd::vector<AZ::Crc32>>();
        }

        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->EBus<DialogueComponentRequestBus>("DialogueComponentRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "SelectAvailableResponseByNum",
                    &DialogueComponentRequests::SelectAvailableResponse,
                    { { { "Choice Number", "The number corresponding to the desired dialogue choice. Starts at '1'." } } })
                ->EventWithBus<DialogueComponentRequestBus>("AbortConversation", &DialogueComponentRequestBus::Events::AbortConversation)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "ContinueConversation", &DialogueComponentRequestBus::Events::ContinueConversation)
                ->EventWithBus<DialogueComponentRequestBus>("EndConversation", &DialogueComponentRequestBus::Events::EndConversation)
                ->EventWithBus<DialogueComponentRequestBus>("GetActiveDialogue", &DialogueComponentRequestBus::Events::GetActiveDialogue)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "GetAvailableResponses", &DialogueComponentRequestBus::Events::GetAvailableResponses)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "TryToStartConversation",
                    &DialogueComponentRequestBus::Events::TryToStartConversation,
                    { { { "Initiator", "The entity starting the conversation." } } })
                // Specifying the specific overloaded SelectDialogue function that takes DialogueData as a parameter.
                ->EventWithBus<DialogueComponentRequestBus, void (DialogueComponentRequestBus::Events::*)(DialogueData const&)>(
                    "SelectDialogue",
                    &DialogueComponentRequestBus::Events::SelectDialogue,
                    { { { "DialogueData", "The dialogue to make active." } } })
                ->EventWithBus<DialogueComponentRequestBus>(
                    "TrySelectDialogueById",
                    &DialogueComponentRequestBus::Events::TryToSelectDialogue,
                    { { { "DialogueId", "The ID of the dialogue to look for and make active." } } });

            behaviorContext->EBus<DialogueComponentNotificationBus>("DialogueComponentNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Handler<BehaviorDialogueComponentNotificationBusHandler>();

            behaviorContext->EBus<GlobalConversationNotificationBus>("GlobalConversationNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Handler<BehaviorGlobalConversationNotificationBusHandler>();

            behaviorContext->Constant("ActiveConversationTag", BehaviorConstant(ActiveConversationTag));
            behaviorContext->Constant("PlayerConversationTag", BehaviorConstant(PlayerConversationTag));
        }
    }

    void DialogueComponent::Init()
    {
        if (!GetEntity()->FindComponent(AZ::TypeId{ TagComponentTypeId }))
        {
            AZLOG_ERROR( // NOLINT
                "Dialogue component [EntityName: %s | EntityId: %s] does not have a TagComponent, which is required! The request bus for "
                "this entity will be disconnected.\n",
                GetNamedEntityId().GetName().data(),
                GetEntityId().ToString().c_str());

            DialogueComponentRequestBus::Handler::BusDisconnect();
        }
    }

    void DialogueComponent::Activate()
    {
        m_conversationAssetRequests = ConversationAssetRefComponentRequestBus::FindFirstHandler(GetEntityId());

        if (m_conversationAssetRequests)
        {
            // Activating without one is fine, if intentional.
            LOG_EntityComponent("LOG_DialogueComponent", *this, "Activated without a ConversationAssetRefComponent.\n");
        }

        // The TagComponent is used to communicate with speakers, so we add our tag to it upon activation.
        // It will need to be removed upon deactivation.
        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequestBus::Events::AddTag, AZ::Crc32(m_config.m_speakerTag));

        DialogueComponentRequestBus::Handler::BusConnect(GetEntityId());
    }

    void DialogueComponent::Deactivate()
    {
        m_conversationAssetRequests = nullptr; // We don't own it.

        // Just in case there's a conversation, we abort on deactivation.
        AbortConversation();

        DialogueComponentRequestBus::Handler::BusDisconnect(GetEntityId());

        // We remove our speaker tag in case our component get removed.
        // TODO: Consider if this is necessary, since someone may opt to use a tag used by other systems.
        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequestBus::Events::RemoveTag, AZ::Crc32(m_config.m_speakerTag));
    }

    auto DialogueComponent::ReadInConfig(AZ::ComponentConfig const* baseConfig) -> bool
    {
        if (auto config = azrtti_cast<DialogueComponentConfig const*>(baseConfig))
        {
            m_config = (*config);
            return true;
        }

        return false;
    }

    auto DialogueComponent::WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const -> bool
    {
        if (auto* const config = azrtti_cast<DialogueComponentConfig*>(outBaseConfig))
        {
            (*config) = m_config;
            return true;
        }
        return false;
    }

    void DialogueComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DialogueService"));
    }

    void DialogueComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("DialogueService"));
    }

    void DialogueComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TagService"));
    }

    void DialogueComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("ConversationAssetRefService"));
    }

    auto DialogueComponent::TryToStartConversation(AZ::EntityId const& initiatingEntityId) -> bool
    {
        if (!m_conversationAssetRequests)
        {
            AZ_Error( // NOLINT
                "DialogueComponent",
                false,
                "A conversation cannot be started because we did not find a ConversationAssetRefComponent when this component was "
                "activated.");

            return false;
        }

        AZ_Info( // NOLINT
            "DialogueComponent",
            "[Entity: '%s'] Trying to start a conversation.\n",
            GetNamedEntityId().GetName().data());

        // Check that we have what we need to start to successfully start a conversation.

        if (m_currentState != DialogueState::Inactive)
        {
            AZ_Warning( // NOLINT
                "DialogueComponent",
                false,
                "Failed to start conversation. Entity '%s' needs to be in the inactive state . \n",
                GetNamedEntityId().GetName().data());

            return false;
        }

        if (m_conversationAssetRequests->CountDialogues() == 0)
        {
            AZ_Warning( // NOLINT
                "DialogueComponent",
                false,
                "Failed to start conversation. Entity '%s' has no dialogues.\n",
                GetNamedEntityId().GetName().data());
            return false;
        }

        if (m_conversationAssetRequests->CountStartingIds() == 0)
        {
            AZ_Warning( // NOLINT
                "DialogueComponent",
                false,
                "Failed to start conversation. Entity '%s' has no starting ids.\n",
                GetNamedEntityId().GetName().data());
            return false;
        }

        m_currentState = DialogueState::Starting;

        AZLOG( // NOLINT
            LOG_FollowConversation,
            "[Entity: '%s'] Entered starting state. Now checking for available starting dialogues.\n",
            GetNamedEntityId().GetName().data());

        auto const startingIds{ m_conversationAssetRequests->CopyStartingIds() };
        auto const dialogues{ m_conversationAssetRequests->CopyDialogues() };

        // We find the first available starting ID and use it to start the conversation.
        for (DialogueId const& startingId : startingIds)
        {
            // DialogueData and DialogueId are different types. We need to search a
            // list of DialogueData for one matching the current DialogueId. To do so,
            // I create a new instance of DialogueData based on the current DialogueId.
            // DialogueData objects are always equal only if they have matching IDs.
            // This allows me to use AZStd::find to search the container of dialogues.
            auto const startingDialogueIter = dialogues.find(DialogueData(startingId));

            // Verify we found one. This should never fail, but just in case.
            if (startingDialogueIter == dialogues.end() || !IsValid(*startingDialogueIter))
            {
                m_currentState = DialogueState::Inactive;

                AZ_Error( // NOLINT
                    "DialogueComponent",
                    false,
                    "[Entity: '%s'] Failed to find an expected dialogue.\n");

                return false;
            }

            if (CheckAvailability(*startingDialogueIter))
            {
                m_currentState = DialogueState::Active;
                LmbrCentral::TagComponentRequestBus::Event(
                    GetEntityId(), &LmbrCentral::TagComponentRequests::AddTag, ActiveConversationTag);

                auto const initiatingEntityIsPlayer = [&initiatingEntityId]() -> bool
                {
                    bool result{};
                    LmbrCentral::TagComponentRequestBus::EventResult(
                        result, initiatingEntityId, &LmbrCentral::TagComponentRequests::HasTag, AZ_CRC_CE("player"));
                    return result;
                }();

                if (initiatingEntityIsPlayer)
                {
                    LmbrCentral::TagComponentRequestBus::Event(
                        GetEntityId(), &LmbrCentral::TagComponentRequests::AddTag, PlayerConversationTag);
                }

                DialogueComponentNotificationBus::Event(
                    GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationStarted, initiatingEntityId);
                GlobalConversationNotificationBus::Broadcast(
                    &GlobalConversationNotificationBus::Events::OnConversationStarted, initiatingEntityId, GetEntityId());

                SelectDialogue(*startingDialogueIter);
                AZ_Info("DialogueComponent", "A conversation was successfully started."); // NOLINT
                return true; // Success - We're only interested in the first available.
            }
        }

        // We failed to start the conversation.
        m_currentState = DialogueState::Inactive;

        AZ_Warning( // NOLINT
            "DialogueComponent",
            false,
            "A conversation failed to be started after checking for available starting IDs.");

        return false;
    }

    void DialogueComponent::AbortConversation()
    {
        m_currentState = DialogueState::Aborting;
        m_activeDialogue.reset();
        m_currentState = DialogueState::Inactive;

        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(),
            &LmbrCentral::TagComponentRequests::RemoveTags,
            AZStd::unordered_set{ ActiveConversationTag, PlayerConversationTag });

        DialogueComponentNotificationBus::Event(GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationAborted);
        GlobalConversationNotificationBus::Broadcast(&GlobalConversationNotificationBus::Events::OnConversationAborted, GetEntityId());
    }

    void DialogueComponent::EndConversation()
    {
        m_currentState = DialogueState::Ending;
        m_activeDialogue.reset();
        m_currentState = DialogueState::Inactive;

        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(),
            &LmbrCentral::TagComponentRequests::RemoveTags,
            AZStd::unordered_set{ ActiveConversationTag, PlayerConversationTag });

        DialogueComponentNotificationBus::Event(GetEntityId(), &DialogueComponentNotificationBus::Events::OnConversationEnded);
        GlobalConversationNotificationBus::Broadcast(&GlobalConversationNotificationBus::Events::OnConversationEnded, GetEntityId());
    }

    void DialogueComponent::SelectDialogue(DialogueData const& dialogueToSelect)
    {
        // Selection should only be possible in 'Active' or 'Starting'.
        if (!(m_currentState == DialogueState::Active || m_currentState == DialogueState::Starting))
        {
            AZ_Error( // NOLINT
                "DialogueComponent",
                false,
                "SelectDialogue should only be called while we're in the 'Active' or 'Starting' state.");
            return;
        }

        // TODO: Review. For now, if an attempt is made to select a dialogue and there are no
        // responses available, we're going to assume the caller wants to end the
        // conversation. Only check if there's an active dialogue because this function
        // can be called when there's no active dialogue, such as when first starting
        // a conversation.
        if (m_activeDialogue && GetDialogueResponseIds(*m_activeDialogue).empty())
        {
            AZLOG(LOG_FollowConversation, "Ending conversation because there are no responses available."); // NOLINT
            EndConversation();
            return;
        }

        if (!IsValid(dialogueToSelect))
        {
            AZ_Assert( // NOLINT
                false,
                "A valid dialogue is needed in order to make a selection.");

            // Abort when given an invalid dialogue.
            AbortConversation();
            return;
        }

        m_activeDialogue = dialogueToSelect;
        m_availableResponses.clear();

        // Check all responses and determine which should be available for use.
        for (DialogueId const& responseId : GetDialogueResponseIds(*m_activeDialogue))
        {
            AZ::Outcome<DialogueData> const responseDialogueOutcome = m_conversationAssetRequests->GetDialogueById(responseId);
            // An invalid ID means we didn't find a dialogue matching the responseId.
            // We can only check valid dialogues, so we skip ahead if invalid.
            if (!responseDialogueOutcome.IsSuccess())
            {
                continue;
            }

            if (CheckAvailability(responseDialogueOutcome.GetValue()))
            {
                m_availableResponses.push_back(responseDialogueOutcome.GetValue());
            }
        }

        // We send the dialogue out. It's considered spoken after this call.
        DialogueComponentNotificationBus::Event(
            GetEntityId(), &DialogueComponentNotificationBus::Events::OnDialogue, *m_activeDialogue, m_availableResponses);

        AZLOG( // NOLINT
            LOG_FollowConversation,
            "[Dialogue: '%s'] \"%s\"",
            GetNamedEntityId().GetName().data(),
            GetDialogueActorText(*m_activeDialogue).data());

        // Since it's considered spoken, we should sent any necessary notifications related to speaking a dialogue.
        // The first thing we want to do is run any provided scripts.
        AZStd::ranges::for_each(
            GetDialogueScriptIds(*m_activeDialogue),
            [](auto const scriptId)
            {
                DialogueScriptRequestBus::Event(AZ::Crc32(scriptId), &DialogueScriptRequestBus::Events::RunDialogueScript);
            });
    }

    auto DialogueComponent::TryToSelectDialogue(DialogueId const dialogueId) -> bool
    {
        auto const getDialogueOutcome = m_conversationAssetRequests->GetDialogueById(dialogueId);
        if (!getDialogueOutcome.IsSuccess())
        {
            LOG_EntityComponent("LOG_FollowConversation", *this, "Failed to select a dialogue using the given DialogueId.");
            return false;
        }

        SelectDialogue(getDialogueOutcome.GetValue());
        return true;
    }

    void DialogueComponent::SelectAvailableResponse(int const responseNumber)
    {
        // responseNumber must begin with the chosen first number (0 or 1)
        if (!(responseNumber >= FirstResponseNumber))
        {
            return;
        }
        // Technically, FirstResponseNumber could be set to anything, such as
        // 15, then choice 15 would be index 0 in our response container.
        // That is obviously ridiculous, so we limit the choice to zero or one.
        // We set the static assert here, instead of at the declaration, so
        // that it is not instinctively changed when changing the constant.
        static_assert(FirstResponseNumber >= 0 && FirstResponseNumber <= 1, "FirstResponseNumber *MUST* be zero or one.");

        if constexpr (FirstResponseNumber == 0)
        {
            // Check 0-based choice is less than the upper bound [0, containerSize - 1].
            if (responseNumber < m_availableResponses.size())
            {
                return;
            }
        }
        else if constexpr (FirstResponseNumber == 1)
        {
            // Check 1-based choice is less than the upper bounds [0, containerSize].
            if (responseNumber <= m_availableResponses.size())
            {
                return;
            }
        }

        // This is why we care if the choice is 0-based or 1-based. We have to adjust our index accordingly.
        SelectDialogue(m_availableResponses[responseNumber - FirstResponseNumber]);
    }

    void DialogueComponent::ContinueConversation()
    {
        // Require active dialogue
        if (!m_activeDialogue)
        {
            return;
        }
        // Calling continue with no available responses should end the conversation normally.
        if (m_availableResponses.empty())
        {
            EndConversation();
            return;
        }
        // Only if the first available response is the same speaker as the active dialogue, select it.
        if (GetDialogueSpeaker(*m_activeDialogue) == GetDialogueSpeaker(*m_availableResponses.begin()))
        {
            SelectDialogue(*m_availableResponses.begin());
            return;
        }

        // FIXME: If the active dialogue's speaker is the player, we automatically choose an NPC response.
        // This is just a workaround until proper NPC response handling is implemented.
        if (GetDialogueSpeaker(*m_activeDialogue) == PlayerSpeakerTag) // @todo make "player" a constant
        {
            auto const firstAvailableResponseIter = m_availableResponses.begin();
            if (firstAvailableResponseIter != m_availableResponses.end() &&
                GetDialogueSpeaker(*firstAvailableResponseIter) != PlayerSpeakerTag)
            {
                SelectDialogue(*firstAvailableResponseIter);
            }
        }
    }

    auto DialogueComponent::CheckAvailability(DialogueData const& dialogueData) -> bool
    {
        // All availability checks must pass for a dialogue to be available.
        bool const isDialogueAvailable = [this, &dialogueData]() -> bool
        {
            // NOTE: A DialogueData is, by default, available, unless a handler explicitly sets it to false.
            AZ::EBusReduceResult<bool, AZStd::logical_and<bool>> result(true);
            AvailabilityRequestBus::EventResult(
                result, GetEntityId(), &AvailabilityRequestBus::Events::IsAvailable, dialogueData.m_availabilityId.GetStringView());
            return result.value;
        }();

        return isDialogueAvailable;
    }

    auto DialogueComponent::CheckAvailability(DialogueId const& dialogueId) -> bool
    {
        auto const getDialogueOutcome = m_conversationAssetRequests->GetDialogueById(dialogueId);
        return getDialogueOutcome.IsSuccess() ? CheckAvailability(getDialogueOutcome.GetValue()) : false;
    }

} // namespace Conversation
