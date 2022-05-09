
#include <ConversationEditor/ConversationDocument.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace ConversationEditor
{

	void ConversationDocument::Reflect(AZ::ReflectContext* context)
	{
		if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serializeContext->Class<ConversationDocument>()
				->Version(0)
				->Field("Graph", &ConversationDocument::m_graphPtr)
				->Field("Comments", &ConversationDocument::m_comments)
				;


			if (AZ::EditContext* editConext = serializeContext->GetEditContext())
			{
				editConext->Class<ConversationDocument>("EditorConversationAsset", "A graph of a conversation.")

					;
			}
		}
	}

}