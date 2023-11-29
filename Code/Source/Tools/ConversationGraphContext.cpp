#include "Tools/ConversationGraphContext.h"

#include "AzCore/Serialization/SerializeContext.h"
#include "GraphModel/Model/GraphContext.h"

namespace ConversationEditor
{
    void ConversationGraphContext::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationGraphContext, GraphModel::GraphContext>()->Version(0);

            serializeContext->RegisterGenericType<ConversationGraphContextPtr>();
        }
    }

    ConversationGraphContext::ConversationGraphContext()
        : GraphContext("Conversation Graph", ".conversationgraph", {})
    {
    }

    ConversationGraphContext::ConversationGraphContext(GraphModel::DataTypeList const& dataTypes)
        : GraphContext("Conversation Graph", ".conversationgraph", dataTypes)
    {
    }

    ConversationGraphContext::~ConversationGraphContext() = default;

} // namespace ConversationEditor
