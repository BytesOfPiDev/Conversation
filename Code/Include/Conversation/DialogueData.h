#pragma once

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Name/Name.h"
#include "AzCore/std/containers/unordered_set.h"
#include "AzCore/std/string/string.h"
#include "ScriptEvents/ScriptEventsAsset.h"

#include "Conversation/ConversationTypeIds.h"
#include "Conversation/ResponseData.h"
#include "Conversation/UniqueId.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    using AvailabilityId = AZStd::string;
    using AvailabilityIdAsName = AZ::Name;

    struct UniqueId;
    struct DialogueData;

    [[nodiscard]] inline auto ToLua(DialogueData const&) -> AZStd::string;

    struct DialogueChunk
    {
    public:
        AZ_TYPE_INFO(DialogueChunk, "{81C04F0E-BA10-4D66-A3E8-0304FB8EB545}"); // NOLINT
        AZ_DEFAULT_COPY_MOVE(DialogueChunk); // NOLINT

        static void Reflect(AZ::ReflectContext* reflect);

        DialogueChunk() = default;
        DialogueChunk(AZStd::string_view data)
            : m_data{ data }
        {
        }

        ~DialogueChunk() = default;

        auto operator==(DialogueChunk const& rhs) const -> bool
        {
            return GetHash() == rhs.GetHash();
        }

        void SetData(AZStd::string_view data)
        {
            m_data = data;
        }

        [[nodiscard]] auto GetData() const -> AZStd::string_view
        {
            return m_data;
        }

        [[nodiscard]] auto GetHash() const -> size_t
        {
            constexpr AZStd::hash<AZStd::string> hasher;
            return hasher(m_data);
        }

    private:
        AZStd::string m_data;
    };

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
        // Attempting to go beyond this will likely result in it not being added.
        static constexpr auto const MaxResponses = 10;
        static constexpr auto const DefaultEntryDelay = 0;
        static constexpr auto const ChunkArraySize = 4;

        AZ_DEFAULT_COPY_MOVE(DialogueData); // NOLINT

        /**
         * @brief Assigns the dialogue an ID if it doesn't already have one.
         *
         * @param dialogueData The dialogue to initialize
         * @return DialogueId The dialogue's ID.
         *
         * @note The ID should generally never be changed once it is set. If that's necessary, it'll have to be done directly on the object.
         */
        static auto InitDialogueId(DialogueData& dialogueData) -> UniqueId
        {
            if (!dialogueData.IsValid())
            {
                dialogueData.m_id = UniqueId::CreateRandomId();
            }

            return dialogueData.m_id;
        }

        constexpr DialogueData() = default;
        explicit DialogueData(bool const autoInitializeId)
            : m_id(autoInitializeId ? UniqueId::CreateRandomId() : UniqueId::CreateInvalidId()){};
        explicit DialogueData(UniqueId const id);
        /**
         * @brief Setup a valid dialogue
         *
         * @warning Ensure that the given id is valid. It is currently enforced, but might not be in the future.
         */
        DialogueData(UniqueId const id, AZStd::string actorText, AZStd::string speaker = {}, AZStd::vector<UniqueId> responses = {});
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
            constexpr auto idFormat = "\tId: %zu\n";
            constexpr auto textFormat = "\tText: %s\n";
            constexpr auto speakerFormat = "\tSpeaker: %s\n";

            AZStd::string description{};
            description += header;
            description += AZStd::string::format(idFormat, GetDialogueId().GetHash()); // NOLINT
            description += AZStd::string::format(textFormat, GetDialogueActorText().data()); // NOLINT
            description += AZStd::string::format(speakerFormat, GetDialogueSpeaker().data()); // NOLINT

            return description;
        }

        [[nodiscard]] auto IsValid() const -> bool
        {
            return m_id.IsValid();
        }

        [[nodiscard]] auto GetId() const -> UniqueId
        {
            return m_id;
        }

        [[nodiscard]] constexpr auto GetDialogueActorText() const -> AZStd::string_view
        {
            return m_actorText;
        }

        [[nodiscard]] constexpr auto GetDialogueSpeaker() const -> AZStd::string_view
        {
            return m_speaker;
        }

        [[nodiscard]] constexpr auto GetResponseIds() const -> AZStd::vector<UniqueId> const&
        {
            return m_responseIds;
        }

        [[nodiscard]] constexpr auto GetDialogueAudioTrigger() const -> AZStd::string_view
        {
            return m_audioTrigger;
        }

        [[nodiscard]] constexpr auto GetDialogueScriptIds() const -> AZStd::vector<AZStd::string> const&
        {
            return m_scriptIds;
        }

        [[nodiscard]] auto GetDialogueAvailabilityId() const -> AvailabilityId
        {
            return m_availabilityId;
        }

        [[nodiscard]] constexpr auto GetDialogueComment() const -> AZStd::string_view
        {
            return m_comment;
        }

        constexpr void SetDialogueAvailabilityId(AvailabilityId const& newAvailabilityId)
        {
            m_availabilityId = newAvailabilityId;
        }

        constexpr void SetDialogueAvailabilityId(AvailabilityIdAsName const& newAvailabilityId)
        {
            m_availabilityId = newAvailabilityId.GetStringView();
        }

        constexpr void SetDialogueActorText(AZStd::string const& actorText)
        {
            m_actorText = actorText;
        }

        constexpr void SetDialogueSpeaker(AZStd::string const& speaker)
        {
            m_speaker = speaker;
        }

        constexpr void SetDialogueAudioTrigger(AZStd::string const& audioTrigger)
        {
            m_audioTrigger = audioTrigger;
        }

        constexpr void SetDialogueEntryDelay(float entryDelay)
        {
            m_entryDelay = entryDelay;
        }

        constexpr void SetDialogueComment(AZStd::string& comment)
        {
            m_comment = comment;
        }

        constexpr void SetDialogueComment(AZStd::string_view comment)
        {
            m_comment = comment;
        }

        [[nodiscard]] constexpr auto CountResponseIds() const -> size_t
        {
            return GetResponseIds().size();
        };

        [[nodiscard]] auto GetDialogueId() const -> UniqueId
        {
            return m_id;
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
        void AddDialogueResponseId(UniqueId const responseId)
        {
            auto& responseIds = GetResponseIds();
            // Only add the response if we're within the limit.
            if (responseId.IsValid() && responseIds.size() < DialogueData::MaxResponses)
            {
                m_responseIds.push_back(responseId);
            }
        }

        void AddResponses(AZStd::span<UniqueId> responses)
        {
            m_responseIds.insert(m_responseIds.end(), responses.begin(), responses.end());
        }

        void AddDialogueResponseId(ResponseData const& responseData)
        {
            // Only add the response if we're within the limit.
            if (CountResponseIds() < DialogueData::MaxResponses)
            {
                m_responseIds.push_back(responseData.m_responseDialogueId);
            }
        }

    private:
        // A script that is run when this dialogue is activated.
        AZ::Data::Asset<ScriptEvents::ScriptEventsAsset> m_script{};
        [[maybe_unused]] AZStd::array<size_t, ChunkArraySize> m_chunkIds{};
        AZStd::vector<UniqueId> m_responseIds{};
        // Contains script IDs that should be executed upon dialogue selection.
        AZStd::vector<AZStd::string> m_scriptIds{};
        AZStd::vector<AZ::Name> m_conditionIds{};
        AZStd::string m_actorText{};
        AZStd::string m_speaker{};
        // The audio trigger to execute upon selection of this dialogue.
        AZStd::string m_audioTrigger{};
        // Any comments from the writers of this dialogue.
        AZStd::string m_comment{};
        AZStd::string m_availabilityId{};
        UniqueId m_id{ UniqueId::CreateInvalidId() };
        float m_entryDelay{ DefaultEntryDelay };
    };

    using DialogueDataPtr = AZStd::shared_ptr<DialogueData>;
    using DialogueDataContainer = AZStd::unordered_set<DialogueData>;

    /**
     * @brief Creates a string containing lua code that creates a matching object in Lua.
     * @param dialogueData The dialogue to create a lua snippet for.
     * @param variableName The variable name that will be set in the script.
     * @param isLocal If the variable should be prefaced with the 'local' Lua keyword.
     *
     * @note This is meant to be used by tools.
     */
    [[nodiscard]] auto ConvertToLua(
        DialogueData const& dialogueData, AZStd::string_view variableName = "tmpConvertToLua", bool isLocal = true) -> AZStd::string;

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
    struct hash<Conversation::UniqueId>
    {
        auto operator()(Conversation::UniqueId const& obj) const -> size_t
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
