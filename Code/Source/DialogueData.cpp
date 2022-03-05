#include <Conversation/DialogueData.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Conversation
{
    void DialogueData::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueData>()
                ->Version(1)
                ->Field("AvailabilityScript", &DialogueData::m_availabilityScript)
                ->Field("ActorText", &DialogueData::m_actorText)
                ->Field("DialogueID", &DialogueData::m_id)
                ->Field("ResponseIds", &DialogueData::m_responseIds)
                ->Field("ActorType", &DialogueData::m_actorType)
                ->Field("Speaker", &DialogueData::m_speaker)
                ->Field("Script", &DialogueData::m_script)
                ->Field("AudioTrigger", &DialogueData::m_audioTrigger);

            serializeContext->RegisterGenericType<DialogueDataPtr>();
            serializeContext->RegisterGenericType<AZStd::vector<DialogueData>>();
            serializeContext->RegisterGenericType<AZStd::unordered_map<DialogueId, DialogueData>>();

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueData>("Dialogue Data", "Data describing a dialogue option.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_availabilityScript, "Availability Script",
                        "Id that will be queried to see if this dialogue is available.")
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &DialogueData::m_actorText, "Actor Text", "What the actor will say.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_speaker, "Speaker",
                        "Represents a specific actor in the conversation.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_script, "Script",
                        "The Id of the script that will run upon this dialogue being chosen.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_audioTrigger, "AudioTrigger",
                        "The trigger for the audio file to play.");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<DialogueData>("Dialogue Data")
                ->Attribute(AZ::Script::Attributes::Category, AZ_CRC("Dialogue System", 0xa63dad5a))
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "dialogue_system")
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Constructor()
                ->Constructor<
                    const DialogueActorType, const DialogueId, const AZStd::string, const AZStd::string, const AZStd::string,
                    const AZStd::string, const AZStd::set<DialogueId>&>()
                ->Property("Availability Script", BehaviorValueProperty(&DialogueData::m_availabilityScript))
                ->Property("Text", BehaviorValueProperty(&DialogueData::m_actorText))
                ->Property("ID", BehaviorValueProperty(&DialogueData::m_id))
                ->Method("AddResponseId", &DialogueData::AddResponseId, { "Response Id" })
                ->Method("GetResponseIds", &DialogueData::GetResponseIds)
                ->Property("Speaker", BehaviorValueProperty(&DialogueData::m_speaker))
                ->Property("AudioTrigger", BehaviorValueProperty(&DialogueData::m_audioTrigger));
        }
    }

    DialogueData::DialogueData(bool generateRandomId)
        : m_id(generateRandomId ? DialogueId::CreateRandom() : DialogueId::CreateNull())
        , m_actorType(DialogueActorType::Invalid)
        , m_actorText("")
        , m_availabilityScript("")
        , m_script("")
        , m_speaker("")
        , m_responseIds()
    {
    }

    DialogueData::DialogueData(
        const DialogueActorType actorType,
        const DialogueId id,
        const AZStd::string actorText,
        const AZStd::string availabilityscript,
        const AZStd::string dialogueScript,
        const AZStd::string speaker,
        const AZStd::set<DialogueId>& responses)
        : m_id(id)
        , m_actorType(actorType)
        , m_actorText(actorText)
        , m_availabilityScript(availabilityscript)
        , m_script(dialogueScript)
        , m_speaker(speaker)
        , m_responseIds(responses)
    {
        // Ensure the ID is always valid.
        if (m_id.IsNull())
        {
            AZ_Printf("DialogueData", "A null ID was passed in. Creating random ID: %s.", m_id.ToString<AZStd::string>().c_str());
            m_id = DialogueId::CreateRandom();
        }
    }

#pragma region ConversationAsset

    void ConversationAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationAsset, AZ::Data::AssetData>()
                ->Version(0)
                ->Field("StartingIds", &ConversationAsset::m_startingIds)
                ->Field("Dialogues", &ConversationAsset::m_dialogues);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<ConversationAsset>(
                        "Conversation Data", "Stores all the dialogue and other information needed to start a conversation.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }
    }

    void ConversationAsset::AddStartingId(const DialogueId& newStartingId)
    {
        // No null Ids are allowed.
        if (newStartingId.IsNull())
        {
            return;
        }

        // Check if it's already in the container.
        auto iterFoundStartingId = AZStd::find_if(
            m_startingIds.begin(), m_startingIds.end(),
            [&newStartingId](const DialogueId& existingId)
            {
                return existingId.GetHash() == newStartingId.GetHash();
            });

        if (iterFoundStartingId != m_startingIds.end())
        {
            return;
        }

        m_startingIds.push_back(newStartingId);
    }

    void ConversationAsset::AddDialogue(const DialogueData& newDialogueData)
    {
        if (!newDialogueData.IsValid() || m_dialogues.contains(newDialogueData.GetId()))
        {
            return;
        }

        m_dialogues[newDialogueData.GetId()] = newDialogueData;
    }

    void ConversationAsset::AddResponseId(const DialogueId& parentDialogueId, const DialogueId& responseDialogueId)
    {
        if (!m_dialogues.contains(parentDialogueId))
        {
            return;
        }

        m_dialogues[parentDialogueId].AddResponseId(responseDialogueId);
    }

    AZ::Outcome<DialogueData> ConversationAsset::GetDialogueById(const DialogueId& dialogueId)
    {
        return m_dialogues.contains(dialogueId) ? AZ::Success(m_dialogues[dialogueId]) : AZ::Outcome<DialogueData>(AZ::Failure());
    }

#pragma endregion
} // namespace Conversation
