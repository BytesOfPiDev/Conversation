#include <ConversationEditor/Common.h>
#include <ConversationEditor/VariableNodePaletteTreeItemTypes.h>
#include <GraphModel/Integration/NodePalette/GraphCanvasNodePaletteItems.h>

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
}