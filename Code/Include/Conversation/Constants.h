#pragma once

namespace Conversation
{
    constexpr auto const* DialogueSystemCategory = "BoP/Conversation";
    constexpr auto const* DialogueSystemModule = "dlgsys";
} // namespace Conversation

namespace ConversationEditor
{
    constexpr auto ConversationCanvasGraphViewSettings = "/O3DE/Atom/GraphView/ViewSettings";
    namespace NodeSettings
    {
        constexpr auto InstructionKey = "instruction";
        constexpr auto NodeTypeKey = "nodeType";

    } // namespace NodeSettings
} // namespace ConversationEditor
