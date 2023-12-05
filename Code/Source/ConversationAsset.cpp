#include "Conversation/ConversationAsset.h"

#include "AzCore/Asset/AssetCommon.h"
#include "AzCore/Asset/AssetSerializer.h"
#include "AzCore/Console/ILogger.h"
#include "AzCore/RTTI/BehaviorContext.h"
#include "AzCore/Script/ScriptContextAttributes.h"
#include "AzCore/Serialization/EditContext.h"
#include "Conversation/Constants.h"
#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueData_incl.h"

namespace Conversation
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(ConversationAsset, "ConversationAsset", ConversationAssetTypeId); // NOLINT
    AZ_RTTI_NO_TYPE_INFO_IMPL(ConversationAsset, AZ::Data::AssetData); // NOLINT
    AZ_CLASS_ALLOCATOR_IMPL(ConversationAsset, AZ::SystemAllocator, 0); // NOLINT

    void ConversationAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationAsset, AZ::Data::AssetData>()
                ->Version(1)
                ->Field("Chunks", &ConversationAsset::m_chunks)
                ->Field("Comment", &ConversationAsset::m_comment)
                ->Field("Dialogues", &ConversationAsset::m_dialogues)
                ->Field("MainScript", &ConversationAsset::m_mainScript)
                ->Field("ResponseData", &ConversationAsset::m_responses)
                ->Field("StartingIds", &ConversationAsset::m_startingIds);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<ConversationAsset>("ConversationAsset", "Stores dialogue and other information needed to start a conversation.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ConversationAsset>("ConversationAsset")
                ->Attribute(AZ::Script::Attributes::Category, DialogueSystemCategory)
                ->Attribute(AZ::Script::Attributes::Module, DialogueSystemModule)
                ->Attribute(AZ::Script::Attributes::ConstructibleFromNil, true)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common);
        }
    }

    void ConversationAsset::AddStartingId(DialogueId const& newStartingId)
    {
        if (!IsValid(newStartingId))
        {
            AZLOG_WARN("Adding a starting ID requires that the ID is not null."); // NOLINT
            return;
        }

        // Check if it's already in the container.
        auto const iterFoundStartingId = AZStd::find_if(
            m_startingIds.begin(), m_startingIds.end(),
            [&newStartingId](DialogueId const& existingId)
            {
                return existingId == newStartingId;
            });

        // Early out if it's already in the container.
        if (iterFoundStartingId != m_startingIds.end())
        {
            AZLOG_WARN("The starting ID is already in the conversation asset."); // NOLINT
            return;
        }

        m_startingIds.push_back(newStartingId);
    }

    void ConversationAsset::AddDialogue(DialogueData const& newDialogueData)
    {
        if (!IsValid(newDialogueData))
        {
            AZ_Warning( // NOLINT
                "ConversationAsset", false,
                "Unable to add dialogue because the ID is invalid! It must be set to a valid ID before being added.\n");
            return;
        }

        if (m_dialogues.contains(newDialogueData))
        {
            AZ_Warning( // NOLINT
                "ConversationAsset", false, "Unable to add dialogue because there is already a dialogue with the same ID.\n");
            return;
        }

        m_dialogues.insert(newDialogueData);
    }

    void ConversationAsset::AddResponse(ResponseData const& responseData)
    {
        if (!IsValid(responseData))
        {
            AZLOG_WARN("Attempt to add invalid response data to a ConversationAsset rejected. Each ID must not be null."); // NOLINT
            return;
        }

        auto iter = AZStd::ranges::find_if(
            m_dialogues,
            [&responseData](DialogueData const& dialogueData) -> bool
            {
                return (dialogueData.m_id == responseData.m_parentDialogueId);
            });

        // We add response data without confirming if we have a DialogueData
        // with the parent DialogueId. We don't require that either exist in
        // our asset.
        m_responses.push_back(responseData);

        // Exit if the parent dialogue wasn't found.
        if (iter == m_dialogues.end())
        {
            return;
        }

        // If it was found, we also add the response directly to the DialogueData.
        AddDialogueResponseId(*iter, responseData);
    }

    auto ConversationAsset::GetDialogueById(DialogueId const& dialogueId) -> AZ::Outcome<DialogueData>
    {
        auto iter = m_dialogues.find(DialogueData(dialogueId));
        return iter != m_dialogues.end() ? AZ::Success(*iter) : AZ::Outcome<DialogueData>(AZ::Failure());
    }

} // namespace Conversation
