#pragma once

#include <GraphModel/Model/GraphContext.h>

namespace ConversationEditor
{
	class ConversationGraphContext : public GraphModel::GraphContext
	{

	public:
		ConversationGraphContext();
		~ConversationGraphContext() override;

		GraphModel::DataTypePtr GetDataTypeForValue(const AZStd::any& value) const override;

	};
}
