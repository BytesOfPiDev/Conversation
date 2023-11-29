#include "Conversation/DialogueComponent.h"

#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Console/IConsole.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/Debug/Trace.h"
#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/Script/ScriptContextAttributes.h"
#include "AzCore/Serialization/SerializeContext.h"
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

namespace Conversation
{
    constexpr auto ActiveConversationTag = AZ_CRC_CE("active_conversation");
    constexpr auto PlayerConversationTag = AZ_CRC_CE("player_conversation");

    AZ_COMPONENT_IMPL(DialogueComponent, "DialogueComponent", DialogueComponentTypeId); // NOLINT

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

        void OnDialogue(const DialogueData& dialogue, const AZStd::vector<DialogueData>& availableResponses) override
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

        void OnResponseAvailable(const DialogueData& availableDialogue) override
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
                ->Version(1)
                ->Field("Config", &DialogueComponent::m_config)
                ->Field("Dialogues", &DialogueComponent::m_dialogues)
                ->Field("DisplayName", &DialogueComponent::m_displayName)
                ->Field("MemoryAsset", &DialogueComponent::m_memoryConversationAsset)
                ->Field("SpeakerIconPath", &DialogueComponent::m_speakerIconPath)
                ->Field("SpeakerTag", &DialogueComponent::m_speakerTag)
                ->Field("StartingIds", &DialogueComponent::m_startingIds);

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
                    "SelectAvailableResponseByNum", &DialogueComponentRequests::SelectAvailableResponse,
                    { { { "Choice Number", "The number corresponding to the desired dialogue choice. Starts at '1'." } } })
                ->EventWithBus<DialogueComponentRequestBus>("AbortConversation", &DialogueComponentRequestBus::Events::AbortConversation)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "ContinueConversation", &DialogueComponentRequestBus::Events::ContinueConversation)
                ->EventWithBus<DialogueComponentRequestBus>("EndConversation", &DialogueComponentRequestBus::Events::EndConversation)
                ->EventWithBus<DialogueComponentRequestBus>("GetActiveDialogue", &DialogueComponentRequestBus::Events::GetActiveDialogue)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "GetAvailableResponses", &DialogueComponentRequestBus::Events::GetAvailableResponses)
                ->EventWithBus<DialogueComponentRequestBus>("GetDialogues", &DialogueComponentRequestBus::Events::GetDialogues)
                ->EventWithBus<DialogueComponentRequestBus>("GetDisplayName", &DialogueComponentRequestBus::Events::GetDisplayName)
                ->EventWithBus<DialogueComponentRequestBus>("GetStartingIds", &DialogueComponentRequestBus::Events::GetStartingIds)
                ->EventWithBus<DialogueComponentRequestBus>(
                    "TryToStartConversation", &DialogueComponentRequestBus::Events::TryToStartConversation,
                    { { { "Initiator", "The entity starting the conversation." } } })
                ->EventWithBus<DialogueComponentRequestBus>(
                    "FindDialogueById", &DialogueComponentRequestBus::Events::FindDialogue, { { { "DialogueId", "" } } })
                ->EventWithBus<DialogueComponentRequestBus>(
                    "DoesDialogueWithIdExist", &DialogueComponentRequestBus::Events::CheckIfDialogueIdExists, { { { "DialogueId", "" } } })
                ->EventWithBus<DialogueComponentRequestBus>(
                    "GetConversationAssets", &DialogueComponentRequestBus::Events::GetConversationAssets)
                ->EventWithBus<DialogueComponentRequestBus>("GetSpeakerTag", &DialogueComponentRequestBus::Events::GetSpeakerTag)
                // Specifying the specific overloaded SelectDialogue function that takes DialogueData as a parameter.
                ->EventWithBus<DialogueComponentRequestBus, void (DialogueComponentRequestBus::Events::*)(const DialogueData&)>(
                    "SelectDialogue", &DialogueComponentRequestBus::Events::SelectDialogue,
                    { { { "DialogueData", "The dialogue to make active." } } })
                // Specifying the specific overloaded SelectDialogue function that takes DialogueId as a parameter.
                ->EventWithBus<DialogueComponentRequestBus, void (DialogueComponentRequestBus::Events::*)(const DialogueId)>(
                    "SelectDialogueById", &DialogueComponentRequestBus::Events::SelectDialogue,
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

            behaviorContext->Constant(
                "ActiveConversationTag",
                []() -> AZ::Crc32
                {
                    return ActiveConversationTag;
                });

            behaviorContext->Constant(
                "PlayerConversationTag",
                []() -> AZ::Crc32
                {
                    return PlayerConversationTag;
                });
        }
    }

    void DialogueComponent::Init()
    {
    }

    void DialogueComponent::Activate()
    {
        AZ_Trace("DialogueComponent", "TestTraceActivate");
        // Combine the starting IDs and dialogues into one container.
        for (AZ::Data::Asset<ConversationAsset>& asset : m_config.m_assets)
        {
            m_startingIds.insert(m_startingIds.end(), asset->GetStartingIds().begin(), asset->GetStartingIds().end());
            m_dialogues.insert(asset->GetDialogues().begin(), asset->GetDialogues().end());
        }

        // The TagComponent is used to communicate with speakers, so we add it as a tag upon activation.
        // It will need to be removed upon deactivation.
        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequestBus::Events::AddTag, AZ::Crc32(m_speakerTag));

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

    auto DialogueComponent::TryToStartConversation(AZ::EntityId const& initiatingEntityId) -> AZStd::optional<AZStd::string>
    {
        AZ_Info( // NOLINT
            "DialogueComponent", "[Entity: '%s'] Trying to start a conversation.\n", GetNamedEntityId().GetName().data());

        // Check that we have what we need to start to successfully start a conversation.

        if (m_currentState != DialogueState::Inactive)
        {
            auto const error = AZStd::string::format(
                "Failed to start conversation. Entity '%s' needs to be in the inactive state . \n", GetNamedEntityId().GetName().data());

            AZ_Warning( // NOLINT
                "DialogueComponent", false, error.data());
            return error;
        }

        if (m_dialogues.empty())
        {
            auto const error =
                AZStd::string::format("Failed to start conversation. Entity '%s' has no dialogues.\n", GetNamedEntityId().GetName().data());

            AZ_Warning( // NOLINT
                "DialogueComponent", false, error.data());
            return error;
        }

        if (m_startingIds.empty())
        {
            auto const error = AZStd::string::format(
                "Failed to start conversation. Entity '%s' has no starting ids.\n", GetNamedEntityId().GetName().data());

            AZ_Warning( // NOLINT
                "DialogueComponent", false, error.data());
            return error;
        }

        m_currentState = DialogueState::Starting;

        AZLOG( // NOLINT
            LOG_FollowConversation, "[Entity: '%s'] Entered starting state. Now checking for available starting dialogues.\n",
            GetNamedEntityId().GetName().data());

        // We find the first available starting ID and use it to start the conversation.
        for (const DialogueId& startingId : m_startingIds)
        {
            // DialogueData and DialogueId are different types. We need to search a
            // list of DialogueData for one matching the current DialogueId. To do so,
            // I create a new instance of DialogueData based on the current DialogueId.
            // DialogueData objects are always equal only if they have matching IDs.
            // This allows me to use AZStd::find to search the container of dialogues.
            const auto startingDialogueIter = m_dialogues.find(DialogueData(startingId));

            // Verify we found one. This should never fail, but just in case.
            if (startingDialogueIter == m_dialogues.end() || !IsValid(*startingDialogueIter))
            {
                m_currentState = DialogueState::Inactive;

                auto const error =
                    AZStd::string::format("[Entity: '%s'] Failed to find an expected dialogue.\n", GetNamedEntityId().GetName().data());

                AZ_Error( // NOLINT
                    "DialogueComponent", false, error.data());

                return error;
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
                AZLOG(LOG_FollowConversation, "A conversation was successfully started."); // NOLINT
                return {}; // Success - We're only interested in the first available.
            }
        }

        // We failed to start the conversation.
        m_currentState = DialogueState::Inactive;

        auto const error = "A conversation failed to be started after checking for available starting IDs.";
        AZ_Printf("DialogueComponent", error);

        return error;
    }

    void DialogueComponent::AbortConversation()
    {
        m_currentState = DialogueState::Aborting;
        m_activeDialogue.reset();
        m_currentState = DialogueState::Inactive;

        LmbrCentral::TagComponentRequestBus::Event(
            GetEntityId(), &LmbrCentral::TagComponentRequests::RemoveTags,
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
            GetEntityId(), &LmbrCentral::TagComponentRequests::RemoveTags,
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
                "DialogueComponent", false, "SelectDialogue should only be called while we're in the 'Active' or 'Starting' state.");
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
                false, "A valid dialogue is needed in order to make a selection.");

            // Abort when given an invalid dialogue.
            AbortConversation();
            return;
        }

        m_activeDialogue = dialogueToSelect;
        m_availableResponses.clear();

        // Check all responses and determine which should be available for use.
        for (DialogueId const& responseId : GetDialogueResponseIds(*m_activeDialogue))
        {
            DialogueData const responseDialogue = FindDialogue(responseId);
            // An invalid ID means we didn't find a dialogue matching the responseId.
            // We can only check valid dialogues, so we skip ahead if invalid.
            if (!IsValid(responseDialogue))
            {
                continue;
            }

            if (CheckAvailability(responseDialogue))
            {
                m_availableResponses.push_back(responseDialogue);
            }
        }

        // We send the dialogue out. It's considered spoken after this call.
        DialogueComponentNotificationBus::Event(
            GetEntityId(), &DialogueComponentNotificationBus::Events::OnDialogue, *m_activeDialogue, m_availableResponses);

        AZLOG( // NOLINT
            LOG_FollowConversation, "[Dialogue: '%s'] \"%s\"", GetNamedEntityId().GetName().data(),
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

    void DialogueComponent::SelectDialogue(DialogueId const dialogueId)
    {
        auto dialogueIter = m_dialogues.find(DialogueData(dialogueId));
        SelectDialogue(dialogueIter != m_dialogues.end() ? *dialogueIter : DialogueData());
    }

    void DialogueComponent::SelectAvailableResponse(int const responseNumber)
    {
        if (responseNumber < 1 || responseNumber > m_availableResponses.size())
        {
            return;
        }

        SelectDialogue(m_availableResponses[responseNumber - 1]);
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

        // If the active dialogue's speaker is the player, we automatically choose an NPC response.
        // This is just a workaround until proper NPC response handling is implemented.
        if (GetDialogueSpeaker(*m_activeDialogue) == "player") // @todo make "player" a constant
        {
            auto const firstAvailableResponseIter = m_availableResponses.begin();
            if (firstAvailableResponseIter != m_availableResponses.end() && GetDialogueSpeaker(*firstAvailableResponseIter) != "player")
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
        return CheckIfDialogueIdExists(dialogueId) ? CheckAvailability(*m_dialogues.find(DialogueData{ dialogueId })) : false;
    }

    auto DialogueComponent::CheckIfDialogueIdExists(DialogueId const& dialogueId) const -> bool
    {
        return m_dialogues.find(DialogueData{ dialogueId }) != m_dialogues.end();
    }

} // namespace Conversation
