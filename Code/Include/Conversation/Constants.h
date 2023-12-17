#pragma once

namespace Conversation
{
    constexpr auto const* DialogueSystemCategory = "BoP/Conversation";
    constexpr auto const* DialogueSystemModule = "dialogue";
} // namespace Conversation

namespace ConversationEditor
{
    // NOTE: Keys *MUST* begin with either "/O3DE/AtomToolsFramework", "/O3DE/Atom/Tools", or "/O3DE/Atom/LY_CMAKE_TARGET"
    // LY_CAKE_TARGET is set in this gem's Code/CMakeLists.txt file
    // @see AtomToolsApplication::Destroy()
    // @note An alternative would be to save the file ourselves, which I'd rather not do unless necessary.
    //
    // clang-format off
    inline constexpr auto ConversationCanvasGraphViewSettingsKey                     = "/O3DE/Atom/ConversationCanvas/GraphView/ViewSettings";
    inline constexpr auto ConversationCanvasSettingsEnablePreviewKey                 = "/O3DE/Atom/ConversationCanvas/EnablePreview";
    inline constexpr auto ConversationCanvasSettingsDocumentInspectorKey             = "/O3DE/Atom/ConversationCanvas/DocumentInspector";
    inline constexpr auto ConversationCanvasSettingsCreateDefaultDocumentOnStartKey  = "/O3DE/Atom/ConversationCanvas/CreateDefaultDocumentOnStart";
    // clang-format on

    namespace NodeSettings
    {
        constexpr auto InstructionKey = "instruction";
        constexpr auto NodeTypeKey = "nodeType";
        constexpr auto NodeTypeValue_Dialogue = "Dialogue";

    } // namespace NodeSettings
} // namespace ConversationEditor
