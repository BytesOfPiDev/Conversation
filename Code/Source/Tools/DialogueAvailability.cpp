#include <DialogueEditor/DialogueAvailability.h>

#include <AzCore/Serialization/EditContext.h>

namespace DialogueEditor
{
    // void CommandDefinition::Reflect(AZ::ReflectContext* context)
    //{
    //	if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
    //	{
    //		serializeContext->Class<CommandDefinition>()
    //			->Version(0)
    //			->Field("Name", &CommandDefinition::m_name)
    //			->Field("Tooltip", &CommandDefinition::m_name)
    //			->Field("Parameters",
    //&CommandDefinition::m_parameterTypes)
    //			;

    //		if (AZ::EditContext* editContext =
    //serializeContext->GetEditContext())
    //		{
    //			editContext->Class<CommandDefinition>("CommandDefinition",
    //"Defines a command.")
    //				->DataElement(AZ::Edit::UIHandlers::Default,
    //&CommandDefinition::m_name, "Name", "The command's name.")
    //				->DataElement(AZ::Edit::UIHandlers::Default,
    //&CommandDefinition::m_tooltip, "Tooltip", "The tooltip.")
    //				//->DataElement(AZ::Edit::UIHandlers::,
    //&CommandDefinition::m_parameters, "Parameters", "Command parameters")
    //				;
    //		}
    //	}
    //}
} // namespace DialogueEditor