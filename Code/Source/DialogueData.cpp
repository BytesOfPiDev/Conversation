#include "Conversation/DialogueData.h"

#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/Script/ScriptContextAttributes.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/Constants.h"

namespace Conversation
{

    void DialogueChunk::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<DialogueChunk>()->Version(1)->Field("Data", &DialogueChunk::m_data);

            if (AZ::EditContext* editContext = serialize->GetEditContext())
            {
                editContext->Class<DialogueChunk>("DialogueChunk", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &DialogueChunk::m_data, "Text Chunk", "");
            }
        }
    }

    void ResponseData::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ResponseData>()
                ->Version(0)
                ->Field("ParentDialogueId", &ResponseData::m_parentDialogueId)
                ->Field("ResponseDialogueId", &ResponseData::m_responseDialogueId);
        }
    }

    void DialogueData::Reflect(AZ::ReflectContext* context)
    {
        ResponseData::Reflect(context);
        DialogueChunk::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueData>()
                ->Version(8) // NOLINT
                ->Field("ActorText", &DialogueData::m_actorText)
                ->Field("AvailabilityId", &DialogueData::m_availabilityId)
                ->Field("AudioTrigger", &DialogueData::m_audioTrigger)
                ->Field("Comment", &DialogueData::m_comment)
                ->Field("DialogueId", &DialogueData::m_id)
                ->Field("ResponseIds", &DialogueData::m_responseIds)
                ->Field("ScriptIds", &DialogueData::m_scriptIds)
                ->Field("Speaker", &DialogueData::m_speaker);

            serializeContext->RegisterGenericType<DialogueDataPtr>();
            serializeContext->RegisterGenericType<AZStd::vector<DialogueData>>();
            serializeContext->RegisterGenericType<AZStd::unordered_set<DialogueData>>();

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<DialogueData>("Dialogue Data", "Data describing a dialogue option.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &DialogueData::m_actorText, "Actor Text", "What the actor will say.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_speaker,
                        "Speaker",
                        "Represents a specific actor in the conversation.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_audioTrigger,
                        "AudioTrigger",
                        "The trigger for the audio file to play.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_scriptIds,
                        "ScriptIds",
                        "Script Ids to be executed upon dialogue selection.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_availabilityId,
                        "AvailabilityId",
                        "Id to be called when determining dialogue availability.")

                    ->DataElement(AZ::Edit::UIHandlers::Button, &DialogueData::m_id, "Id", "");
            }
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<DialogueData>("Dialogue Data")
                ->Attribute(AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::LessThan)
                ->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::LessEqualThan)
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(AZ::Script::Attributes::EnableAsScriptEventReturnType, true)
                ->Constructor()
                ->Property("Text", BehaviorValueProperty(&DialogueData::m_actorText))
                ->Property("ID", &DialogueData::GetDialogueId, nullptr)
                ->Property("Speaker", BehaviorValueProperty(&DialogueData::m_speaker))
                ->Property("AudioTrigger", BehaviorValueProperty(&DialogueData::m_audioTrigger))
                ->Property("ResponseIds", &DialogueData::GetResponseIds, nullptr)
                ->Property("ScriptIds", BehaviorValueGetter(&DialogueData::m_scriptIds), nullptr);
        }
    }

    DialogueData::DialogueData(UniqueId const id, AZStd::string actorText, AZStd::string speaker, AZStd::vector<UniqueId> responses)
        : m_id(id)
        , m_actorText(AZStd::move(actorText))
        , m_speaker(AZStd::move(speaker))
        , m_responseIds(AZStd::move(responses))
    {
        // Ensure the ID is always valid.
        if (m_id.IsValid())
        {
            AZ_Error("DialogueData", false, "A null ID was passed in. Creating random ID: %zu.", m_id.GetHash());
            m_id = UniqueId::CreateRandomId();
        }
    }

    DialogueData::DialogueData(UniqueId const id)
        : m_id(id)
    {
    }

    [[nodiscard]] auto ConvertToLua(DialogueData const& dialogueData, AZStd::string_view variableName, bool isLocal) -> AZStd::string
    {
        AZStd::string luaSource{};
        AZStd::string::format("{%s}{%s} = DialogueData()", (isLocal ? "local " : ""), variableName.data());

        return luaSource;
    }

} // namespace Conversation
