#include "Tools/ConversationGraphContext.h"

#include "AzCore/Serialization/SerializeContext.h"
#include "Conversation/ConversationTypeIds.h"
#include "GraphModel/Model/GraphContext.h"
#include "Tools/DataTypes.h"

namespace ConversationEditor
{
    void ConversationGraphContext::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext
                ->Class<ConversationGraphContext, GraphModel::GraphContext>()
                ->Version(0);

            serializeContext
                ->RegisterGenericType<ConversationGraphContextPtr>();
        }
    }

    ConversationGraphContext::ConversationGraphContext()
        : GraphContext(SystemName, ModuleExtension, {})
    {
    }

    ConversationGraphContext::ConversationGraphContext(
        GraphModel::DataTypeList const& dataTypes)
        : GraphContext(SystemName, ModuleExtension, dataTypes)
    {
    }

    ConversationGraphContext::~ConversationGraphContext() = default;

} // namespace ConversationEditor
