
#include <ConversationSystemComponent.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/Serialization/SerializeContext.h>
#include <Conversation/AvailabilityBus.h>
#include <Conversation/ConversationBus.h>
#include <Conversation/DialogueComponent.h>
#include <Conversation/DialogueScript.h>
#include <DialogueLibrary.h>

namespace Conversation
{
    class BehaviorConversationNotificationBusHandler
        : public ConversationNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorConversationNotificationBusHandler,
            "{D5680C71-684A-451E-823F-DEE4F17D1DA9}",
            AZ::SystemAllocator,
            OnConversationStarted,
            OnConversationEnded,
            OnConversationAborted,
            OnChoiceAvailable,
            OnChoiceSelected,
            OnDialogue);

        void OnConversationStarted(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationStarted, entity);
        }

        void OnConversationEnded(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationEnded, entity);
        }

        void OnConversationAborted(const AZ::EntityId& entity) override
        {
            Call(FN_OnConversationAborted, entity);
        }

        void OnChoiceAvailable(const AZStd::vector<DialogueData>& choices) override
        {
            Call(FN_OnChoiceAvailable, choices);
        }

        void OnChoiceSelected() override
        {
            Call(FN_OnChoiceSelected);
        }

        void OnDialogue(const DialogueData& dialogueData) override
        {
            Call(FN_OnDialogue, dialogueData);
        }
    };

    class BehaviorDialogueScriptRequestBusHandler
        : public DialogueScriptRequestBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(
            BehaviorDialogueScriptRequestBusHandler, "{168DA145-68E2-4D49-BCE7-3BAE5589C3D1}", AZ::SystemAllocator, RunDialogueScript);

        void RunDialogueScript() override
        {
            Call(FN_RunDialogueScript);
        }
    };

    void ConversationSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        DialogueData::Reflect(context);
        ConversationAsset::Reflect(context);
        DialogueLibrary::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ConversationSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext
                    ->Class<ConversationSystemComponent>("Conversation", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<AvailabilityNotificationBus>("AvailabilityNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Handler<BehaviorAvailabilityNotificationBusHandler>();

            const AZ::BehaviorParameterOverrides startConversationEntityIdParam = {
                "EntityId", "The Id of an entity that contains a conversation component that will be used to start the conversation."
            };

            // startConversationParams.

            behaviorContext->EBus<ConversationRequestBus>("ConversationRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Event("StartConversation", &ConversationRequestBus::Events::StartConversation, { startConversationEntityIdParam })
                ->Event("EndConversation", &ConversationRequestBus::Events::EndConversation)
                ->Event("AbortConversation", &ConversationRequestBus::Events::AbortConversation)
                ->Event("SelectResponseById", &ConversationRequestBus::Events::SelectResponseById)
                ->Event("SelectResponseByNumber", &ConversationRequestBus::Events::SelectResponseByNumber);

            behaviorContext->EBus<ConversationNotificationBus>("ConversationNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Handler<BehaviorConversationNotificationBusHandler>();
        }
    }

    void ConversationSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ConversationService"));
    }

    void ConversationSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void ConversationSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    ConversationSystemComponent::ConversationSystemComponent()
    {
        if (ConversationInterface::Get() == nullptr)
        {
            ConversationInterface::Register(this);
        }
    }

    ConversationSystemComponent::~ConversationSystemComponent()
    {
        if (ConversationInterface::Get() == this)
        {
            ConversationInterface::Unregister(this);
        }
    }

    void ConversationSystemComponent::AbortConversation()
    {
        if (m_currentConversationStatus != ConversationStatus::Active)
        {
            return;
        }

        AZ_Assert(m_currentConversationData, "Current conversation data pointer is null! It should be valid in the current state.");
        ConversationNotificationBus::Broadcast(
            &ConversationNotificationBus::Events::OnConversationAborted, m_currentConversationData->OwningEntity);

        m_currentConversationStatus = ConversationStatus::Aborting;
        m_currentConversationAsset.Reset();
        m_currentConversationData = nullptr;
        m_currentConversationStatus = ConversationStatus::Inactive;
    }

    void ConversationSystemComponent::EndConversation()
    {
    }

    void ConversationSystemComponent::StartConversation(const AZ::EntityId entityId)
    {
        // Refuse to start a conversation if the system is not currently inactive.
        if (m_currentConversationStatus != ConversationStatus::Inactive)
        {
            return;
        }

        // We set the status to starting so external code can know we're in the middle
        // of setting up the conversation. It lets them know not to trust any information
        // while in this state.
        m_currentConversationStatus = ConversationStatus::Starting;
        m_currentConversationData = AZStd::make_unique<ActiveConversationData>();
        m_currentConversationData->OwningEntity = entityId;

        DialogueComponentRequestBus::EventResult(
            m_currentConversationAsset, entityId, &DialogueComponentRequestBus::Events::GetConversationData);

        // Check each starting id and store the ones available for selection.
        AZStd::vector<DialogueData> availableDialogues = GetAvailableDialogues(m_currentConversationAsset->GetStartingIds());

        AZ_Warning(
            "ConversationSystemComponent", !availableDialogues.empty(), "No starting dialogues are available. Cannot start conversation.");
        if (availableDialogues.empty())
        {
            m_currentConversationStatus = ConversationStatus::Inactive;
            return;
        }

        // For now, we're going to choose the first one.
        // Later, we may use some logic to decide which to use when multiple are available.
        const DialogueId chosenDialogueId = availableDialogues[0].GetId();

        // Try to retrieve the dialogue using the id. Should never fail, but potentially could.
        const auto outcome = m_currentConversationAsset->GetDialogueById(chosenDialogueId);

        if (!outcome.IsSuccess())
        {
            // We failed to get everything we needed to start a conversation. Reset everything.
            m_currentConversationAsset.Reset();
            m_currentConversationData = nullptr;
            m_currentConversationStatus = ConversationStatus::Inactive;
            return;
        }

        m_currentConversationStatus = ConversationStatus::Active;
        SendDialogue(outcome.GetValue());
    }

    void ConversationSystemComponent::SelectResponseByNumber(const size_t choiceNumber)
    {
        if (m_currentConversationStatus != ConversationStatus::Active)
        {
            return;
        }

        AZ_Assert(m_currentConversationData, "No active conversation data is available.");

        /**
         * The choiceNumber starts at 1, meaning the user wants to pick choice number one.
         * This is for readability when writing or scripting code to select a choice on screen.
         *
         * The code after this will minus one to keep it in line with C++ indexes. For now,
         * we need to make sure choiceNumber is in a range of acceptable values; 1 <=> container size.
         */
        if (choiceNumber < 1 || choiceNumber > m_currentConversationData->AvailableResponses.size())
        {
            return;
        }

        // We minus one to line up with C++ indexes.
        const size_t choiceIndex = choiceNumber - 1;

        SelectResponse(m_currentConversationData->AvailableResponses[choiceIndex - 1]);
    }

    void ConversationSystemComponent::SelectResponseById(const DialogueId& dialogueId)
    {
        AZ_Assert(!dialogueId.IsNull(), "The dialogue id to select must not be null.");
        auto selectedDialogueOutcome = m_currentConversationAsset->GetDialogueById(dialogueId);

        AZ_Assert(selectedDialogueOutcome.IsSuccess(), "Unable to find a dialogue that matches the given ID.");
        if (!selectedDialogueOutcome.IsSuccess())
        {
            return;
        }

        SelectResponse(selectedDialogueOutcome.GetValue());
    }

    void ConversationSystemComponent::Init()
    {
        AZ::EnvironmentVariable<ScriptCanvas::NodeRegistry> nodeRegistryVariable =
            AZ::Environment::FindVariable<ScriptCanvas::NodeRegistry>(ScriptCanvas::s_nodeRegistryName);
        if (nodeRegistryVariable)
        {
            ScriptCanvas::NodeRegistry& nodeRegistry = nodeRegistryVariable.Get();
            Conversation::DialogueLibrary::InitNodeRegistry(nodeRegistry);
        }
        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        m_conversationAssetHandler = AZStd::make_unique<ConversationAssetHandler>(
            "Conversation Asset", "Conversation System", ".conversation", AZ::AzTypeInfo<ConversationAsset>::Uuid(), serializeContext);
        AZ_Assert(m_conversationAssetHandler, "Unable to create conversation asset handler.");
    }

    void ConversationSystemComponent::Activate()
    {
        const AZ::Data::AssetType conversationAssetTypeId = azrtti_typeid<ConversationAsset>();
        if (!AZ::Data::AssetManager::Instance().GetHandler(conversationAssetTypeId))
        {
            AZ_Assert(
                m_conversationAssetHandler,
                "Conversation asset handler is null! It should have been created in the Init function already.");
            m_conversationAssetHandler->Register();
        }

        ConversationRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void ConversationSystemComponent::Deactivate()
    {
        AbortConversation();

        if (!AZ::Data::AssetManager::Instance().IsReady() && m_conversationAssetHandler)
        {
            m_conversationAssetHandler->Unregister();
        }

        AZ::TickBus::Handler::BusDisconnect();
        ConversationRequestBus::Handler::BusDisconnect();
    }

    void ConversationSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

    void ConversationSystemComponent::SelectResponse(const DialogueData& responseDialogueData)
    {
        AZ_Assert(responseDialogueData.IsValid(), "Attempting to select an invalid dialogue data as a response.");
        if (!responseDialogueData.IsValid())
        {
            return;
        }

        SendDialogue(responseDialogueData);
    }

    void ConversationSystemComponent::SendDialogue(const DialogueData& dialogueToSend)
    {
        m_currentConversationData->CurrentlyActiveDialogue = dialogueToSend;
        m_currentConversationData->AvailableResponses = GetAvailableDialogues(dialogueToSend.GetResponseIds());

        ConversationNotificationBus::Broadcast(&ConversationNotificationBus::Events::OnDialogue, dialogueToSend);
        DialogueScriptRequestBus::Event(dialogueToSend.GetId(), &DialogueScriptRequestBus::Events::RunDialogueScript);
        // @todo Delay sending choices.
        ConversationNotificationBus::Broadcast(
            &ConversationNotificationBus::Events::OnChoiceAvailable, m_currentConversationData->AvailableResponses);
    }

    AZStd::vector<DialogueData> ConversationSystemComponent::GetAvailableDialogues(const AZStd::set<DialogueId>& responseIds)
    {
        AZStd::vector<DialogueData> availableDialogues;

        for (const DialogueId& responseId : responseIds)
        {
            // Dialogues are available by default. The only time one will not be available is if an
            // availability check was setup somewhere, which does some logic that returns true or false.
            // Remember, the following variable is only modified if the EBus call is successful, so it
            // stays true unless something listening to the signal modifies it.
            bool isAvailableResult = true;
            AvailabilityNotificationBus::EventResult(
                isAvailableResult, responseId, &AvailabilityNotificationBus::Events::OnAvailabilityCheck);
            // The AZ::Outcome lets us know if a dialogue was found with the id we give.
            AZ::Outcome<DialogueData> dialogueDataOutcome =
                isAvailableResult ? m_currentConversationAsset->GetDialogueById(responseId) : AZ::Failure();

            if (!dialogueDataOutcome.IsSuccess())
            {
                continue;
            }

            availableDialogues.push_back(dialogueDataOutcome.GetValue());
        }

        return AZStd::move(availableDialogues);
    }

} // namespace Conversation
