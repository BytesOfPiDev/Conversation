#pragma once

#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/std/any.h>
#include <GraphModel/Model/DataType.h>
#include <GraphModel/Model/Slot.h>

namespace ConversationEditor
{
	namespace CommonSlotDisplayNames
	{
		static const char* AVAILABILITYSCRIPT = "ActiveScript";
		static const char* ACTORTEXT = "Text";
		static const char* DIALOGUEID = "Id";
		static const char* DIALOGUEDATA = "DialogueData";
		static const char* DIALOGUESCRIPT = "DialogueScriptId";
		static const char* ENTER = "Enter";
		static const char* EXIT = "Exit";
		static const char* OUTPUTDIALOGUEID = "OutputId";
		static const char* SPEAKER = "Speaker";
	}

	namespace CommonSlotNames
	{
		static const char* AVAILABILITYSCRIPT = "AvailabilityScript";
		static const char* ACTORTEXT = "ActorText";
		static const char* DIALOGUEID = "DialogueId";
		static const char* DIALOGUEDATA = "DialogueData";
		static const char* DIALOGUESCRIPT = "DialogueScript";
		static const char* ENTER = "Enter";
		static const char* EXIT = "Exit";
		static const char* OUTPUTDIALOGUEID = "OutputDialogueId";
		static const char* SPEAKER = "Speaker";
	}

	namespace CommonSlotDescriptions
	{
		static const char* AVAILABILITYSCRIPT = "An identifier for a script or bus Id that will answer if this dialogue is visible/usable.";
		static const char* ACTORTEXT = "The text that will show up on screen.";
		static const char* DIALOGUEID = "The globally unique identifier for this specific dialogue.";
		static const char* DIALOGUEDATA = "Stores data related to a specific dialogue node.";
		static const char* DIALOGUESCRIPT = "An identifier for a script that runs when the dialogue is used.";
		static const char* ENTER = "";
		static const char* EXIT = "";
		static const char* OUTPUTDIALOGUEID = DIALOGUEID;
		static const char* SPEAKER = "The tag representing a specific entity that is not the player. An empty/null tag represents the player.";
	}

	namespace NodeTypes
	{
		enum class DocumentEditorDataTypesEnum : GraphModel::DataType::Enum
		{
			InvalidEntity = 0,  // Need a special case data type for the AZ::EntityId::InvalidEntityId to handle the default value since we are re-using the AZ::EntityId type in several node data types

			Dialogue,
			String,
			DialogueId,
			Number
		};

		static const AZ::Uuid InvalidEntityTypeId = "{3BB6E8E9-5525-4B88-A406-DBDE20E9FA5E}";		static const AZ::Uuid RootTypeId = "{78F43C22-AEFC-44FB-A94C-F5B6DC87BFA8}";
	}

	class DocumentEditorDataType
		: public GraphModel::DataType
	{
	public:
		AZ_CLASS_ALLOCATOR(DocumentEditorDataType, AZ::SystemAllocator, 0);
		AZ_RTTI(DocumentEditorDataType, "{D1FB14EC-D1C1-4CB0-BD81-C1A66EF84A3F}", GraphModel::DataType);

		static void Reflect(AZ::ReflectContext* context);

		DocumentEditorDataType() = default;
		DocumentEditorDataType(Enum typeEnum, AZ::Uuid typeUuid, AZStd::any defaultValue, AZStd::string_view typeDisplayName, AZStd::string_view cppTypeName);
	};
}
