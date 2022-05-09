#include <ConversationEditor/DataTypes.h>

namespace ConversationEditor
{
	DocumentEditorDataType::DocumentEditorDataType(Enum typeEnum, AZ::Uuid typeUuid, AZStd::any defaultValue, AZStd::string_view typeDisplayName, AZStd::string_view cppTypeName)
		: GraphModel::DataType(typeEnum, typeUuid, defaultValue, typeDisplayName, cppTypeName)
	{

	}

	void DocumentEditorDataType::Reflect(AZ::ReflectContext* context)
	{
		AZ_UNUSED(context);
	}
}