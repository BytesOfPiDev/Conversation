#include <AzCore/Math/Crc.h>
#include <AzCore/std/any.h>
#include <AzCore/std/containers/compressed_pair.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <Conversation/DialogueData.h>
#include <GraphModel/Model/DataType.h>
#include <ConversationEditor/Common.h>
#include <ConversationEditor/DataTypes.h>
#include <ConversationEditor/ConversationGraphContext.h>

namespace ConversationEditor
{
	ConversationGraphContext::ConversationGraphContext()
        : GraphContext("Conversation Graph System", ".conversationgraph", {})
	{
			const AZ::EntityId invalidEntity;
			const AZStd::any defaultValue(invalidEntity);
			const AZStd::string cppName = "AZ::EntityId";

			/**
			 * @todo FIx default values. Right now, they're all Invalid entity IDs. I had mirrored functionality from Landscape Canvas, but it doesn't work for us
			 * We need to have different actual types or at least values (in which case we use GetDataTypeForValue to deduce type, so we can say what can connect to what.
			 * Each Node we currently have will need to be adjusted accordingly inside their RegisterSlots function.
			 */
			m_dataTypes.push_back(
				AZStd::make_shared<DocumentEditorDataType>(
					static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::InvalidEntity), 
					NodeTypes::InvalidEntityTypeId,
					defaultValue, 
					"InvalidEntity", 
					cppName));


			auto dialogueDataPtrGenericTypeInfo =
				AZ::SerializeGenericTypeInfo<Conversation::DialogueDataPtr>::GetGenericInfo();

			m_dataTypes.push_back(
				AZStd::make_shared<DocumentEditorDataType>(
					static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::Dialogue),
					dialogueDataPtrGenericTypeInfo->GetSpecializedTypeId(), AZStd::make_any<Conversation::DialogueDataPtr>(),
					"DialogueDataPtr",
					dialogueDataPtrGenericTypeInfo->TYPEINFO_Name()));

			// Construct basic data types
			m_dataTypes.push_back(
				AZStd::make_shared<DocumentEditorDataType>(
					static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::String),
					AZ::AzTypeInfo<AZStd::string>::Uuid(),
					AZStd::make_any<AZStd::string>(), 
					"String", 
					"AZStd::string"));

			m_dataTypes.push_back(
				AZStd::make_shared<DocumentEditorDataType>(
					static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::DialogueId),
                            AZ::AzTypeInfo<Conversation::DialogueId>::Uuid(),
					AZStd::make_any<AZ::Uuid>(),
					"DialogueId",
					"AZ::Uuid"));

			m_dataTypes.push_back(
				AZStd::make_shared<DocumentEditorDataType>(
					static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::Number),
					AZ::AzTypeInfo<int>::Uuid(),
					AZStd::make_any<int>(0),
					"Number",
					"int"));
		
	}

	ConversationGraphContext::~ConversationGraphContext() = default;



	GraphModel::DataTypePtr ConversationGraphContext::GetDataTypeForValue(const AZStd::any& value) const
	{
		// If the value is an AZ::EntityId::InvalidEntityId return our special
		// case node data type to handle the default values.
		const AZ::Uuid type = value.type();
		if (type == azrtti_typeid<AZ::EntityId>())
		{
			auto entityId = AZStd::any_cast<AZ::EntityId>(value);
			if (!entityId.IsValid())
			{
				return GetDataType(static_cast<GraphModel::DataType::Enum>(NodeTypes::DocumentEditorDataTypesEnum::InvalidEntity));
			}
		}

		return GraphContext::GetDataTypeForValue(value);
	}

} // namespace ConversationEditor
