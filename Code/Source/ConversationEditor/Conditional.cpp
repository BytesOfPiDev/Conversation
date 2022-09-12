#include <ConversationEditor/Conditional.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <ConversationEditor/Settings.h>

namespace ConversationEditor
{
    void Conditional::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<Conditional>()
                ->Version(1)
                ->Field("FunctionId", &Conditional::m_functionId)
                ->Field("Parameters", &Conditional::m_parameters);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<Conditional>("Conditional", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }
    ConditionManager::ConditionManager()
    {
        auto editorSettings =
            AZ::UserSettings::CreateFind<ConversationEditorSettings>(ConversationEditorSettingsId, AZ::UserSettings::CT_LOCAL);

        AZStd::vector<ConditionalFunction> functions;

        ScriptEvents::ScriptEvent const* const scriptEvent = editorSettings->GetScriptEvents().GetDefinition();
        BuildFunctions(scriptEvent);
    }
    AZStd::vector<ConditionalFunction> ConditionManager::BuildFunctions(ScriptEvents::ScriptEvent const* const scriptEvent)
    {
        for (const ScriptEvents::Method& method : scriptEvent->GetMethods())
        {
            method.GetParameters().front().GetTypeProperty();
        }

        return AZStd::vector<ConditionalFunction>();
    }
} // namespace ConversationEditor
