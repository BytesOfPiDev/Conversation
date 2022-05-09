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
                ->Field("ActorText", &DialogueData::m_actorText)
                ->Field("DialogueID", &DialogueData::m_id)
                ->Field("ResponseIds", &DialogueData::m_responseIds)
                ->Field("Speaker", &DialogueData::m_speaker)
                ->Field("AudioTrigger", &DialogueData::m_audioTrigger);

            serializeContext->RegisterGenericType<DialogueDataPtr>();
            serializeContext->RegisterGenericType<AZStd::vector<DialogueData>>();
            serializeContext->RegisterGenericType<AZStd::unordered_map<DialogueId, DialogueData>>();
            serializeContext->RegisterGenericType<AZStd::vector<AZ::Crc32>>();
            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueData>("Dialogue Data", "Data describing a dialogue option.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &DialogueData::m_actorText, "Actor Text", "What the actor will say.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_speaker, "Speaker",
                        "Represents a specific actor in the conversation.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &DialogueData::m_audioTrigger, "AudioTrigger",
                        "The trigger for the audio file to play.");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<DialogueData>("Dialogue Data")
                ->Attribute(AZ::Script::Attributes::Category, "Dialogue System")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "dialogue_system")
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::LessThan)
                ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::LessEqualThan)
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventReturnType, true)
                ->Constructor()
                ->Property("Text", BehaviorValueProperty(&DialogueData::m_actorText))
                ->Property("ID", BehaviorValueProperty(&DialogueData::m_id))
                ->Method("AddResponseId", &DialogueData::AddResponseId, "Response Id" )
                ->Method("GetResponseIds", &DialogueData::GetResponseIds)
                ->Property("Speaker", BehaviorValueProperty(&DialogueData::m_speaker))
                ->Property("AudioTrigger", BehaviorValueProperty(&DialogueData::m_audioTrigger));

            behaviorContext->Class<AZStd::vector<AZ::Crc32>>("Crc Vector")
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventReturnType, true);
        }
    }

    DialogueData::DialogueData(bool generateRandomId)
        : m_id(generateRandomId ? DialogueId::CreateRandom() : DialogueId::CreateNull())
        , m_actorText("")
        , m_speaker("")
        , m_responseIds()
    {

    }

    DialogueData::DialogueData(
        const DialogueId id, const AZStd::string actorText, const AZStd::string speaker, const AZStd::set<DialogueId>& responses)
        : m_id(id)
        , m_actorText(actorText)
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

} // namespace Conversation

