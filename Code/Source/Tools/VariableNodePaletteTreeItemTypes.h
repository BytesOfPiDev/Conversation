#pragma once

#include <AzCore/Memory/SystemAllocator.h>

#include <GraphCanvas/Editor/AssetEditorBus.h>
#include <GraphCanvas/Widgets/NodePalette/TreeItems/DraggableNodePaletteTreeItem.h>

namespace ConversationEditor
{
	class VariableCategoryNodePaletteTreeItem
		: public GraphCanvas::NodePaletteTreeItem
	{
	public:
		AZ_CLASS_ALLOCATOR(VariableCategoryNodePaletteTreeItem, AZ::SystemAllocator, 0);
		VariableCategoryNodePaletteTreeItem(AZStd::string_view displayName);
		~VariableCategoryNodePaletteTreeItem() = default;

	private:

		void PreOnChildAdded(GraphCanvasTreeItem* item) override;
	};
}
