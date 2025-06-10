#pragma once

#include "AzCore/Memory/SystemAllocator.h"
#include "AzCore/RTTI/ReflectContext.h"
#include "GraphModel/Model/GraphContext.h"

namespace ConversationCanvas
{
    /**
     * @class ConversationGraphContext
     **/
    class ConversationGraphContext : public GraphModel::GraphContext
    {
    public:
        AZ_RTTI(
            ConversationGraphContext,
            "{32C647DD-E0BD-452C-BC67-C8A81FE40FB1}",
            GraphModel::GraphContext);
        AZ_CLASS_ALLOCATOR(ConversationGraphContext, AZ::SystemAllocator, 0);
        AZ_DISABLE_COPY_MOVE(ConversationGraphContext);

        static constexpr auto SystemName = "Conversation Graph";
        static constexpr auto ModuleExtension = ".conversationgraph";

        static void Reflect(AZ::ReflectContext* context);

        ConversationGraphContext();
        ConversationGraphContext(GraphModel::DataTypeList const& dataTypes);
        ~ConversationGraphContext() override;
    };

    using ConversationGraphContextPtr =
        AZStd::shared_ptr<ConversationGraphContext>;

} // namespace ConversationCanvas
