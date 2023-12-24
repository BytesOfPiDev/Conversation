#pragma once

#include "GraphCanvas/Editor/EditorTypes.h"
#include "GraphModel/Model/DataType.h"

namespace ConversationEditor
{
    static GraphCanvas::EditorId const AssetEditorId =
        AZ_CRC("ConversationEditorApp", 0x9d94345e);

    static char const* SAVE_IDENTIFIER = "DialogueDocumentEditorSaveIdentifier";
} // namespace ConversationEditor
