#include <Conversation/ConversationAsset.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Conversation
{
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
                        "ConversationAsset", "Stores dialogue and other information needed to start a conversation.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "");
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<ConversationAsset>("ConversationAsset");
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

        m_startingIds.insert(newStartingId);
    }

    void ConversationAsset::AddDialogue(const DialogueData& newDialogueData)
    {
        if (!newDialogueData.IsValid() || m_dialogues.contains(newDialogueData.GetId()))
        {
            return;
        }

        m_dialogues.insert(newDialogueData);
    }

    void ConversationAsset::AddResponseId(const DialogueId& parentDialogueId, const DialogueId& responseDialogueId)
    {
        auto iter = m_dialogues.find(parentDialogueId);

        if (iter == m_dialogues.end())
        {
            return;
        }

        iter->AddResponseId(responseDialogueId);
    }

    AZ::Outcome<DialogueData> ConversationAsset::GetDialogueById(const DialogueId& dialogueId)
    {
        auto iter = m_dialogues.find(dialogueId);
        return iter != m_dialogues.end() ? AZ::Success(*iter) : AZ::Outcome<DialogueData>(AZ::Failure());
    }

} // namespace Conversation
