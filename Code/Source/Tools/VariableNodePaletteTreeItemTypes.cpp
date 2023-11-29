#include <Tools/VariableNodePaletteTreeItemTypes.h>

#include <GraphModel/Integration/NodePalette/GraphCanvasNodePaletteItems.h>
#include <Tools/Common.h>

namespace ConversationEditor
{
    VariableCategoryNodePaletteTreeItem::VariableCategoryNodePaletteTreeItem(AZStd::string_view displayName)
        : GraphCanvas::NodePaletteTreeItem(displayName, AssetEditorId)
    {
    }

    void VariableCategoryNodePaletteTreeItem::PreOnChildAdded(GraphCanvasTreeItem* item)
    {
        GraphCanvas::NodePaletteTreeItem::PreOnChildAdded(item);
    }
} // namespace ConversationEditor
