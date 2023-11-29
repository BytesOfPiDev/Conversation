#pragma once

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/Name/Name.h"
#include "AzCore/RTTI/RTTIMacros.h"
#include "AzCore/std/containers/unordered_set.h"
#include "AzCore/std/string/string.h"
#include "Conversation/ConversationTypeIds.h"

#include "Conversation/Util.h"

namespace AZ
{
    class ReflectContext;
}

namespace Conversation
{
    using AvailabilityId = AZStd::string;

    struct DialogueId;
    struct DialogueData;
    struct ResponseData;

    [[nodiscard]] inline auto CreateRandomDialogueId() -> DialogueId;
    [[nodiscard]] inline auto ToString(DialogueId const& dialogueId) -> AZStd::string;
    [[nodiscard]] inline auto ToString(DialogueData const&) -> AZStd::string;

    [[nodiscard]] inline constexpr auto IsValid(DialogueId const& dialogueId) -> bool;
    [[nodiscard]] inline constexpr auto IsValid(DialogueData const& dialogueData) -> bool;
    [[nodiscard]] inline constexpr auto IsValid(ResponseData const& responseData) -> bool;

    [[nodiscard]] inline constexpr auto GetDialogueActorText(DialogueData const& dialogueData) -> AZStd::string_view;
    [[nodiscard]] inline constexpr auto GetDialogueAudioTrigger(DialogueData const& dialogueData) -> AZStd::string_view;
    [[nodiscard]] inline constexpr auto GetDialogueComment(DialogueData const& dialogueData) -> AZStd::string_view;
    [[nodiscard]] inline constexpr auto GetDialogueId(DialogueData const& dialogueData) -> DialogueId;
    [[nodiscard]] inline constexpr auto GetDialogueSpeaker(DialogueData const& dialogueData) -> AZStd::string_view;
    [[nodiscard]] inline constexpr auto GetDialogueResponseIds(DialogueData const& dialogueData) -> AZStd::vector<DialogueId> const&;
    [[nodiscard]] inline constexpr auto ModifyDialogueResponseIds(DialogueData& dialogueData) -> AZStd::vector<DialogueId>&;
    [[nodiscard]] inline constexpr auto GetDialogueScriptIds(DialogueData const& dialogueData) -> const AZStd::vector<AZStd::string>&;
    [[nodiscard]] inline auto GetDialogueAvailabilityId(DialogueData const& dialogueData) -> AZ::Name;

    inline constexpr void SetDialogueActorText(DialogueData& dialogueData, const AZStd::string& actorText);
    inline constexpr void SetDialogueAvailabilityId(DialogueData& dialogueData, AZ::Name const& newAvailabilityId);
    inline constexpr void SetDialogueAvailabilityId(DialogueData& dialogueData, AZStd::string const& newAvailabilityId);
    inline constexpr void SetDialogueComment(DialogueData& dialogueData, AZStd::string_view comment);
    inline constexpr void SetDialogueSpeaker(DialogueData& dialogueData, const AZStd::string& speaker);
    inline constexpr void SetDialogueAudioTrigger(DialogueData& dialogueData, const AZStd::string& audioTrigger);
    inline constexpr void SetDialogueEntryDelay(DialogueData& dialogueData, float entryDelay);
    inline constexpr void SetDialogueComment(DialogueData& dialogueData, AZStd::string& comment);

    inline constexpr void AddDialogueResponseId(DialogueData& dialogueData, DialogueId const responseId);
    inline constexpr void AddDialogueResponseId(ResponseData const responseData);

    /**
     * @brief Assigns the dialogue an ID if it doesn't already have one.
     *
     * @param dialogueData The dialogue to initialize
     * @return DialogueId The dialogue's ID.
     *
     * @note The ID should generally never be changed once it is set. If that's necessary, it'll have to be done directly on the object.
     */
    inline auto InitDialogueId(DialogueData& dialogueData) -> DialogueId;

    [[nodiscard]] inline auto ToLua(DialogueData const&) -> AZStd::string;

    struct DialogueChunk
    {
    public:
        AZ_TYPE_INFO(DialogueChunk, "81C04F0E-BA10-4D66-A3E8-0304FB8EB545"); // NOLINT
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

    struct DialogueId
    {
    public:
        DialogueId() = default;

        explicit DialogueId(AZStd::string_view name)
            : m_id(AZ::Uuid::CreateName(name)){};

        explicit DialogueId(AZ::Uuid uuid)
            : m_id(uuid){};

        AZ_TYPE_INFO(DialogueId, "68AE77C6-9865-47DE-8BFE-D6D67663C5DC"); // NOLINT
        AZ_CLASS_ALLOCATOR(DialogueId, AZ::SystemAllocator, 0); // NOLINT

        static void Reflect(AZ::ReflectContext* context);

        /**
         * Determine if this dialogue's ID is equal to another dialogue ID.
         *
         * This is useful for keeping DialogueId's instead of entire DialogueData objects
         * and then subsequently using the ID to find a matching DialogueData.
         *
         * @param left The first DialogueId to compare.
         * @param right The second DialogueId to compare to.
         * @return Whether left and right are equivalent.
         */
        [[nodiscard]] auto operator==(DialogueId const& other) const -> bool
        {
            return m_id == other.m_id;
        }

        [[nodiscard]] auto operator!=(DialogueId const& other) const -> bool
        {
            return m_id != other.m_id;
        }

        [[nodiscard]] auto operator>(DialogueId const& other) const -> bool
        {
            return m_id > other.m_id;
        }

        [[nodiscard]] auto operator<(DialogueId const& other) const -> bool
        {
            return m_id < other.m_id;
        }

        auto operator<=(DialogueId const& other) const -> bool
        {
            return (m_id <= other.m_id);
        }

        auto operator>=(DialogueId const& other) const -> bool
        {
            return (m_id >= other.m_id);
        }

        AZ::Uuid m_id;
    };

    /**
     * Represents the data needed to create a response.
     **/
    struct ResponseData
    {
        AZ_TYPE_INFO(ResponseData, "AEC51FC7-A91F-40D6-8EBA-59D0EADBAA4C"); // NOLINT
        static void Reflect(AZ::ReflectContext* context);

        // The Id of the dialogue to create a response to.
        DialogueId m_parentDialogueId;
        // The Id of the dialogue that will become a response.
        DialogueId m_responseDialogueId;
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

        constexpr DialogueData() = default;
        explicit DialogueData(bool const autoInitializeId)
            : m_id(autoInitializeId ? CreateRandomDialogueId() : DialogueId()){};
        explicit DialogueData(DialogueId const id);
        DialogueData(DialogueId const id, AZStd::string actorText, AZStd::string speaker = {}, AZStd::vector<DialogueId> responses = {});
        ~DialogueData() = default;

        constexpr auto operator==(const DialogueData& other) const -> bool
        {
            return this->m_id == other.m_id;
        }

        void OnResetId();

        constexpr auto operator==(const DialogueId& dialogueId) const -> bool
        {
            return m_id == dialogueId;
        }

        constexpr auto operator<(const DialogueData& other) const -> bool
        {
            return m_id < other.m_id;
        }

        constexpr auto operator>(const DialogueData& other) const -> bool
        {
            return m_id > other.m_id;
        }

        constexpr auto operator<=(const DialogueData& other) const -> bool
        {
            return !(m_id > other.m_id);
        }

        constexpr auto operator>=(const DialogueData& other) const -> bool
        {
            return !(m_id < other.m_id);
        }

        DialogueId m_id{};
        AZStd::string m_actorText{};
        AZStd::string m_speaker{};
        AZStd::vector<DialogueId> m_responseIds{};
        // The audio trigger to execute upon selection of this dialogue.
        AZStd::string m_audioTrigger{};
        // Any comments from the writers of this dialogue.
        AZStd::string m_comment{};
        float m_entryDelay{ DefaultEntryDelay };
        // Contains script IDs that should be executed upon dialogue selection.
        AZStd::vector<AZStd::string> m_scriptIds{};
        AZStd::vector<AZ::Name> m_conditionIds{};
        AZ::Name m_availabilityId{};
        AZStd::array<size_t, ChunkArraySize> m_chunkIds{};
    };

    using DialogueDataPtr = AZStd::shared_ptr<DialogueData>;
    using DialogueDataContainer = AZStd::unordered_set<DialogueData>;

    [[nodiscard]] inline auto CreateRandomDialogueId() -> DialogueId
    {
        return DialogueId(AZ::Uuid::CreateRandom());
    }

    [[nodiscard]] inline auto ToString(DialogueId const& dialogueId) -> AZStd::string
    {
        return dialogueId.m_id.ToString<AZStd::string>();
    }

    [[nodiscard]] inline auto ToString(DialogueData const& dialogueData) -> AZStd::string
    {
        AZStd::string description{};
        description += AZStd::string("DialogueData [ID: '") + ToString(dialogueData.m_id) + "']\n";
        description += AZStd::string("\tText: ") + dialogueData.m_actorText.c_str() + "\n";
        description += AZStd::string("\tSpeaker: ") + dialogueData.m_speaker.c_str() + "\n";

        return description;
    }

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
    struct hash<Conversation::DialogueId>
    {
        auto operator()(Conversation::DialogueId const& obj) const -> size_t
        {
            return obj.m_id.GetHash();
        }
    };

    template<>
    struct hash<Conversation::DialogueData>
    {
        auto operator()(Conversation::DialogueData const& obj) const -> size_t
        {
            return obj.m_id.m_id.GetHash();
        }
    };

} // namespace AZStd
