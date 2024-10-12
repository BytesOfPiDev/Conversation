#pragma once

#include "GraphCanvas/Editor/EditorTypes.h"

namespace ConversationCanvas
{
    static GraphCanvas::EditorId const AssetEditorId =
        AZ_CRC("ConversationEditorApp", 0x9d94345e);

    static auto* SAVE_IDENTIFIER = "DialogueDocumentEditorSaveIdentifier";

    // NOTE: Keys *MUST* begin with either "/O3DE/AtomToolsFramework",
    // "/O3DE/Atom/Tools", or "/O3DE/Atom/LY_CMAKE_TARGET" LY_CAKE_TARGET is set
    // in this gem's Code/CMakeLists.txt file
    // @see AtomToolsApplication::Destroy()
    // @note An alternative would be to save the file ourselves, which I'd
    // rather not do unless necessary.
    //
    // clang-format off
    constexpr auto ConversationCanvasGraphViewSettingsKey                     = "/O3DE/Atom/ConversationCanvas/GraphView/ViewSettings";
    constexpr auto ConversationCanvasSettingsDocumentInspectorKey             = "/O3DE/Atom/ConversationCanvas/DocumentInspector";
    constexpr auto ConversationCanvasSettingsCreateDefaultDocumentOnStartKey  = "/O3DE/Atom/ConversationCanvas/CreateDefaultDocumentOnStart";
    constexpr auto ConversationCanvasSettingsForceDeleteGeneratedFilesKey     = "/O3DE/Atom/ConversationCanvas/ForceDeleteGeneratedFiles";
    // clang-format on

    namespace NodeSettings
    {
        constexpr auto InstructionsKey = "instructions";
        constexpr auto NodeTypeKey = "nodeType";
        constexpr auto NodeTypeValue_Dialogue = "Dialogue";

    } // namespace NodeSettings

    namespace Settings
    {
        constexpr auto FormatLua =
            "/O3DE/Atom/ConversationCanvas/Lua/EnableFormatting";
        constexpr auto LuaFormatter =
            "/O3DE/Atom/ConversationCanvas/Lua/Formatter";

    } // namespace Settings
} // namespace ConversationCanvas
