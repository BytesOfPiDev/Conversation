#pragma once

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Name/Name.h"
#include "AzCore/std/containers/unordered_set.h"
#include "AzCore/std/string/string.h"

#include "Conversation/ConversationTypeIds.h"
#include "Conversation/DialogueChunk.h"
#include "Conversation/ResponseData.h"
#include "Conversation/UniqueId.h"
#include "DialogueAudioControl.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    using AvailabilityId = UniqueId;

    struct DialogueData;

    /**
     * @brief Represents a piece of dialogue in a conversation.
     *
     */
    struct DialogueData
    {
    public:
        AZ_TYPE_INFO(DialogueData, DialogueDataTypeId); // NOLINT
        AZ_CLASS_ALLOCATOR(DialogueData, AZ::SystemAllocator, 0); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        // There's a limit to the maximum amount of dialogue responses.
        // Attempting to go beyond this will likely result in it not being
        // added.
        //
        // @todo Review limit.
        static constexpr auto MaxResponses{ 10 };
        static constexpr auto DefaultEntryDelay{ 0 };
        static constexpr auto DefaultExitDelay{ 0 };

        AZ_DEFAULT_COPY_MOVE(DialogueData); // NOLINT

        /**
         * @brief Assigns the dialogue an ID if it doesn't already have one.
         *
         * @param dialogueData The dialogue to initialize
         * @return DialogueId The dialogue's ID.
         *
         * @note The ID should generally never be changed once it is set. Not
         * currently enforced.
         */
        static auto InitId(DialogueData& dialogueData) -> UniqueId
        {
            if (!dialogueData.IsValid())
            {
                dialogueData.m_id = UniqueId::CreateRandomId();
            }

            return dialogueData.m_id;
        }

        constexpr DialogueData() = default;
        explicit DialogueData(UniqueId const id);
        explicit DialogueData(bool generateRandomId)
            : m_id(
                  generateRandomId ? UniqueId::CreateRandomId() : UniqueId{}){};
        /**
         * @brief Setup a valid dialogue
         *
         * @warning Ensure that the given id is valid. It is currently enforced,
         * but might not be in the future.
         */
        DialogueData(
            UniqueId const id,
            AZStd::string actorText,
            AZStd::string speaker = {},
            AZStd::vector<UniqueId> responses = {});

        ~DialogueData() = default;

        constexpr auto operator==(DialogueData const& other) const -> bool
        {
            return this->m_id == other.m_id;
        }

        constexpr auto operator==(UniqueId const& dialogueId) const -> bool
        {
            return m_id == dialogueId;
        }

        [[nodiscard]] inline auto ToString() const -> AZStd::string
        {
            constexpr auto header = "[[ Dialogue Data ]]";
            constexpr auto idFormat = "\tId: %u\n";
            constexpr auto textFormat = "\tText: %s\n";
            constexpr auto speakerFormat = "\tSpeaker: %s\n";

            AZStd::string description{};
            description += header;
            description +=
                AZStd::string::format(idFormat, GetId().GetHash()); // NOLINT
            description += AZStd::string::format(
                textFormat, GetShortText().data()); // NOLINT
            description += AZStd::string::format(
                speakerFormat, GetSpeaker().data()); // NOLINT

            return description;
        }

        [[nodiscard]] constexpr auto IsValid() const -> bool
        {
            return m_id.IsValid();
        }

        [[nodiscard]] constexpr auto GetId() const -> UniqueId
        {
            return m_id;
        }

        [[nodiscard]] auto GetName() const -> AZ::Name
        {
            return m_id.GetName();
        }

        [[nodiscard]] constexpr auto GetResponseIds() const
            -> AZStd::vector<UniqueId> const&
        {
            return m_responseIds;
        }

        [[nodiscard]] auto GetAudioControl() const -> DialogueAudioControl
        {
            return m_audioControl;
        }

        [[nodiscard]] constexpr auto GetAvailabilityId() const -> UniqueId
        {
            return m_availabilityId;
        }

        [[nodiscard]] constexpr auto GetComment() const -> AZStd::string_view
        {
            return m_comment;
        }

        constexpr void SetAvailabilityId(AvailabilityId newAvailabilityId)
        {
            m_availabilityId = newAvailabilityId;
        }

        constexpr void SetAvailabilityId(AZStd::string_view newAvailabilityId)
        {
            m_availabilityId = UniqueId::CreateNamedId(newAvailabilityId);
        }

        [[nodiscard]] constexpr auto GetShortText() const -> AZStd::string_view
        {
            return m_shortText;
        }

        constexpr void SetShortText(AZStd::string_view actorText)
        {
            m_shortText = actorText;
        }

        [[nodiscard]] constexpr auto GetSpeaker() const -> AZStd::string_view
        {
            return m_speaker;
        }

        constexpr void SetSpeaker(AZStd::string_view speaker)
        {
            m_speaker = speaker;
        }

        constexpr void SetAudioControl(DialogueAudioControl const& audioTrigger)
        {
            m_audioControl = audioTrigger;
        }

        constexpr void SetEntryDelay(float entryDelay)
        {
            m_entryDelay = entryDelay;
        }

        constexpr void SetComment(AZStd::string& comment)
        {
            m_comment = comment;
        }

        constexpr void SetComment(AZStd::string_view comment)
        {
            m_comment = comment;
        }

        [[nodiscard]] constexpr auto CountResponseIds() const -> size_t
        {
            return GetResponseIds().size();
        };

        /**
         * @brief Add a response ID that can follow this dialogue.
         *
         * The response ID should match an existing DialogueData located
         * elsewhere in the dialogue asset that houses this dialogue, but it
         * doesn't have to.
         *
         * TODO: Review having a limit
         *
         * There is a maximum amount of responses that can be added to a
         * dialogue. This limit is set in DialogueData::MaxResponses Attempting
         * to go beyond that will result in the response not being added. Don't
         * do it.
         *
         * @param responseId The DialogueId of the potential reponse.
         *
         * @note It is possible to add a DialogueId that doesn't exist here
         * unlike adding responses through the ConversationAsset interface.
         *
         * @see ConversationAsset
         */
        constexpr void AddResponseId(UniqueId const responseId)
        {
            auto& responseIds = GetResponseIds();
            // Only add the response if we're within the limit.
            if (responseId.IsValid() &&
                responseIds.size() < DialogueData::MaxResponses)
            {
                m_responseIds.push_back(responseId);
            }
        }

        constexpr void AddResponses(AZStd::span<UniqueId const> responses)
        {
            if (CountResponseIds() + responses.size() >
                DialogueData::MaxResponses)
            {
                AZ_Warning(
                    "DialogueData",
                    false,
                    "Failed to add responses, as it would total more than the "
                    "allowed responses.");

                return;
            }

            m_responseIds.insert(
                m_responseIds.end(), responses.begin(), responses.end());
        }

        constexpr void AddDialogueResponseId(ResponseData const& responseData)
        {
            // Only add the response if we're within the limit.
            if (CountResponseIds() < DialogueData::MaxResponses)
            {
                m_responseIds.push_back(responseData.m_responseDialogueId);
            }
        }

        constexpr void SetChunk(DialogueChunk const& chunk)
        {
            // FIXME: While still developing, setting a chunk when the short
            // text is empty will also assign the chunk's value to the short
            // text. This will change once chunks are fully implemented.
            if (m_shortText.empty())
            {
                m_shortText = chunk.GetData();
            }

            m_dialogueChunk = chunk;
        }

        [[nodiscard]] constexpr auto GetChunkAsText() -> AZStd::string_view
        {
            return m_dialogueChunk.GetData();
        }

        [[nodiscard]] auto GetChunk() const -> DialogueChunk
        {
            return m_dialogueChunk;
        }

        constexpr void SetCinematicId(AZ::Name const& cinematicId)
        {
            m_cinematicId = cinematicId;
        }

        [[nodiscard]] auto GetCinematicId() const -> AZ::Name
        {
            return m_cinematicId;
        }

    private:
        DialogueChunk m_dialogueChunk{};
        AZStd::vector<UniqueId> m_responseIds{};
        AZStd::string m_shortText{};
        AZStd::string m_speaker{};
        // The audio trigger to execute upon selection of this dialogue.
        DialogueAudioControl m_audioControl{};
        AZ::Name m_cinematicId{};
        // Any comments from the writers of this dialogue.
        AZStd::string m_comment{};
        UniqueId m_availabilityId{};
        UniqueId m_id{ UniqueId::CreateInvalidId() };
        float m_entryDelay{ DefaultEntryDelay };
        float m_exitDelay{ DefaultExitDelay };
    };

    using DialogueDataPtr = AZStd::shared_ptr<DialogueData>;
    using DialogueDataContainer = AZStd::unordered_set<DialogueData>;

    /**
     * @brief Creates a string containing lua code that creates a matching
     * object in Lua.
     * @param dialogueData The dialogue to create a lua snippet for.
     * @param variableName The variable name that will be set in the script.
     * @param isLocal If the variable should be prefaced with the 'local' Lua
     * keyword.
     *
     * @note This is meant to be used by tools.
     */
    [[nodiscard]] auto ConvertToLua(
        DialogueData const& dialogueData,
        AZStd::string_view variableName = "tmpConvertToLua",
        bool isLocal = true) -> AZStd::string;

} // namespace Conversation

namespace AZStd
{
    template<>
    struct hash<Conversation::DialogueChunk>
    {
        auto operator()(Conversation::DialogueChunk const& obj) const -> size_t
        {
            return obj.GetHash();
        }
    };

    template<>
    struct hash<Conversation::DialogueData>
    {
        auto operator()(Conversation::DialogueData const& obj) const -> size_t
        {
            return obj.GetId().GetHash();
        }
    };

} // namespace AZStd
