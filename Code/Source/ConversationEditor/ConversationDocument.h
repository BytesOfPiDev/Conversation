#pragma once

#include <ConversationEditor/ConversationGraph.h>

namespace ConversationEditor
{
	/**
	 * Represents a dialogue document used for creating conversations.
	 * 
	 * @see Dialogue::DialogueAsset
	 */
	struct ConversationDocument
	{
		AZ_TYPE_INFO(ConversationDocument, "{0FACF214-F7CD-484A-B618-500DDC22231B}");
		AZ_CLASS_ALLOCATOR(ConversationDocument, AZ::SystemAllocator, 0);

		static void Reflect(AZ::ReflectContext* context);

		ConversationDocument() = default;
		~ConversationDocument() = default;

		AZStd::shared_ptr<ConversationGraph> m_graphPtr;
		AZStd::string m_comments = "Weee!";
	};
} // namespace ConversationEditor
