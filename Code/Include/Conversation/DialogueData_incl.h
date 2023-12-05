#pragma once

#include "Conversation/DialogueData.h"

namespace Conversation
{
    [[nodiscard]] inline constexpr auto IsValid(DialogueId const& dialogueId) -> bool
    {
        return !dialogueId.m_id.IsNull();
    }

    [[nodiscard]] inline constexpr auto IsValid(DialogueData const& dialogueData) -> bool
    {
        return IsValid(dialogueData.m_id);
    }

    [[nodiscard]] inline constexpr auto IsValid(ResponseData const& responseData) -> bool
    {
        return !responseData.m_parentDialogueId.m_id.IsNull() && !responseData.m_responseDialogueId.m_id.IsNull();
    }

    [[nodiscard]] inline constexpr auto GetDialogueId(DialogueData const& dialogueData) -> DialogueId
    {
        return dialogueData.m_id;
    }

    inline auto InitDialogueId(DialogueData& dialogueData) -> DialogueId
    {
        if (!IsValid(dialogueData))
        {
            dialogueData.m_id = CreateRandomDialogueId();
        }

        return dialogueData.m_id;
    }

    [[nodiscard]] inline constexpr auto GetDialogueComment(DialogueData const& dialogueData) -> AZStd::string_view
    {
        return dialogueData.m_comment;
    }

    inline constexpr void SetDialogueComment(DialogueData& dialogueData, AZStd::string_view comment)
    {
        dialogueData.m_comment = comment;
    }

    [[nodiscard]] inline constexpr auto GetDialogueActorText(DialogueData const& dialogueData) -> AZStd::string_view
    {
        return dialogueData.m_actorText;
    }

    [[nodiscard]] inline constexpr auto GetDialogueSpeaker(DialogueData const& dialogueData) -> AZStd::string_view
    {
        return dialogueData.m_speaker;
    }

    [[nodiscard]] inline constexpr auto GetDialogueResponseIds(DialogueData const& dialogueData) -> AZStd::vector<DialogueId> const&
    {
        return dialogueData.m_responseIds;
    }

    [[nodiscard]] inline constexpr auto ModifyDialogueResponseIds(DialogueData& dialogueData) -> AZStd::vector<DialogueId>&
    {
        return dialogueData.m_responseIds;
    }

    [[nodiscard]] inline constexpr auto GetDialogueAudioTrigger(DialogueData const& dialogueData) -> AZStd::string_view
    {
        return dialogueData.m_audioTrigger;
    }

    [[nodiscard]] inline constexpr auto GetDialogueScriptIds(DialogueData const& dialogueData) -> const AZStd::vector<AZStd::string>&
    {
        return dialogueData.m_scriptIds;
    }

    [[nodiscard]] inline auto GetDialogueAvailabilityId(DialogueData const& dialogueData) -> AZ::Name
    {
        return dialogueData.m_availabilityId;
    }

    inline constexpr void SetDialogueAvailabilityId(DialogueData& dialogueData, AZStd::string const& newAvailabilityId)
    {
        dialogueData.m_availabilityId = newAvailabilityId;
    }

    inline constexpr void SetDialogueAvailabilityId(DialogueData& dialogueData, AZ::Name const& newAvailabilityId)
    {
        dialogueData.m_availabilityId = newAvailabilityId;
    }

    inline constexpr void SetDialogueActorText(DialogueData& dialogueData, const AZStd::string& actorText)
    {
        dialogueData.m_actorText = actorText;
    }

    inline constexpr void SetDialogueSpeaker(DialogueData& dialogueData, const AZStd::string& speaker)
    {
        dialogueData.m_speaker = speaker;
    }

    /**
     * @brief Add a response ID that can follow this dialogue.
     *
     * The response ID should match an existing DialogueData located elsewhere
     * in the dialogue asset that houses this dialogue, but it doesn't have to.
     *
     * TODO: Review having a limit
     *
     * There is a maximum amount of responses that can be added to a dialogue.
     * This limit is set in DialogueData::MaxResponses Attempting to go beyond
     * that will result in the response not being added. Don't do it.
     *
     * @param responseId The DialogueId of the potential reponse.
     *
     * @note It is possible to add a DialogueId that doesn't exist here
     * unlike adding responses through the ConversationAsset interface.
     *
     * @see ConversationAsset
     */
    inline constexpr void AddDialogueResponseId(DialogueData& dialogueData, DialogueId const responseId)
    {
        auto& responseIds = GetDialogueResponseIds(dialogueData);
        // Only add the response if we're within the limit.
        if (!responseId.m_id.IsNull() && responseIds.size() < DialogueData::MaxResponses)
        {
            dialogueData.m_responseIds.push_back(responseId);
        }
    }

    inline constexpr void AddDialogueResponseId(DialogueData& dialogueData, ResponseData const responseData)
    {
        // Only add the response if we're within the limit.
        if (GetDialogueResponseIds(dialogueData).size() < DialogueData::MaxResponses)
        {
            dialogueData.m_responseIds.push_back(responseData.m_responseDialogueId);
        }
    }

    inline constexpr void SetDialogueAudioTrigger(DialogueData& dialogueData, const AZStd::string& audioTrigger)
    {
        dialogueData.m_audioTrigger = audioTrigger;
    }

    constexpr void SetDialogueEntryDelay(DialogueData& dialogueData, float entryDelay)
    {
        dialogueData.m_entryDelay = entryDelay;
    }

    constexpr void SetDialogueComment(DialogueData& dialogueData, AZStd::string& comment)
    {
        dialogueData.m_comment = comment;
    }

} // namespace Conversation