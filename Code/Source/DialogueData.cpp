#include "Conversation/DialogueData.h"

#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/Script/ScriptContextAttributes.h"
#include "AzCore/Serialization/EditContext.h"
#include "AzCore/Serialization/EditContextConstants.inl"
#include "AzCore/Serialization/SerializeContext.h"

#include "Conversation/Constants.h"

namespace Conversation
{

    void ResponseData::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ResponseData>()
                ->Version(0)
                ->Field("ParentDialogueId", &ResponseData::m_parentDialogueId)
                ->Field(
                    "ResponseDialogueId", &ResponseData::m_responseDialogueId);
        }
    }

    void DialogueData::Reflect(AZ::ReflectContext* context)
    {
        ResponseData::Reflect(context);

        if (auto* serializeContext =
                azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DialogueData>()
                ->Version(8) // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                ->Field("ActorText", &DialogueData::m_shortText)
                ->Field("AvailabilityId", &DialogueData::m_availabilityId)
                ->Field("AudioTrigger", &DialogueData::m_audioTrigger)
                ->Field("Comment", &DialogueData::m_comment)
                ->Field("Chunk", &DialogueData::m_dialogueChunk)
                ->Field("DialogueId", &DialogueData::m_id)
                ->Field("ResponseIds", &DialogueData::m_responseIds)
                ->Field("Speaker", &DialogueData::m_speaker);

            serializeContext->RegisterGenericType<DialogueDataPtr>();
            serializeContext
                ->RegisterGenericType<AZStd::vector<DialogueData>>();
            serializeContext
                ->RegisterGenericType<AZStd::unordered_set<DialogueData>>();

            if (AZ::EditContext* editContext =
                    serializeContext->GetEditContext())
            {
                editContext
                    ->Class<DialogueData>(
                        "Dialogue Data", "Data describing a dialogue option.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility,
                        AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_shortText,
                        "Short Text",
                        "A very short description of what will be said. e.g. "
                        "'Agree with her'")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &DialogueData::m_dialogueChunk,
                        "Text Chunk",
                        "The entirety of the text that will be said.")
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
                        &DialogueData::m_availabilityId,
                        "AvailabilityId",
                        "Id to be called when determining dialogue "
                        "availability.")

                    ->DataElement(
                        AZ::Edit::UIHandlers::Button,
                        &DialogueData::m_id,
                        "Id",
                        "");
            }
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<DialogueData>("Dialogue Data")
                ->Attribute(
                    AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(
                    AZ::Script::Attributes::Scope,
                    AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(
                    AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(
                    AZ::Script::Attributes::Operator,
                    AZ::Script::Attributes::OperatorType::LessThan)
                ->Attribute(
                    AZ::Script::Attributes::Operator,
                    AZ::Script::Attributes::OperatorType::LessEqualThan)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventParamType, true)
                ->Attribute(
                    AZ::Script::Attributes::EnableAsScriptEventReturnType, true)
                ->Constructor()
                ->Property(
                    "Text",
                    &DialogueData::GetShortText,
                    &DialogueData::SetShortText)
                ->Property("ID", &DialogueData::GetId, nullptr)
                ->Property(
                    "Speaker",
                    &DialogueData::GetSpeaker,
                    &DialogueData::SetSpeaker)
                ->Property(
                    "AudioTrigger",
                    &DialogueData::GetAudioTrigger,
                    &DialogueData::SetAudioTrigger)
                ->Property(
                    "ResponseIds", &DialogueData::GetResponseIds, nullptr)
                ->Property(
                    "Chunk", &DialogueData::GetChunk, &DialogueData::SetChunk)
                ->Property(
                    "Dialogue Chunk", &DialogueData::GetChunkAsText, nullptr);
        }
    }

    DialogueData::DialogueData(UniqueId const id)
        : m_id(id)
    {
    }

    [[nodiscard]] auto ConvertToLua(
        DialogueData const& /*dialogueData*/,
        AZStd::string_view variableName,
        bool isLocal) -> AZStd::string
    {
        AZStd::string luaSource{};
        AZStd::string::format(
            "{%s}{%s} = DialogueData()",
            (isLocal ? "local " : ""),
            variableName.data());

        return luaSource;
    }

} // namespace Conversation
