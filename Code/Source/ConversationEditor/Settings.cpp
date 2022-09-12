#include <ConversationEditor/Settings.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace ConversationEditor
{
    void ConversationEditorSettings::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ConversationEditorSettings, AZ::UserSettings>()
                ->Version(1)
                ->Field("EnableDebug", &ConversationEditorSettings::m_enableDebug)
                ->Field("ScriptAssets", &ConversationEditorSettings::m_editorScriptEvents);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<ConversationEditorSettings>("Conversation Editor Preferences", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &ConversationEditorSettings::m_editorScriptEvents);
            }
        }
    }

    ConversationEditorSettings::ConversationEditorSettings()
        : m_enableDebug(false)
    {
    }

    AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> CreateSettingsPropertyGroup()
    {
        AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> propertyGroup;
        propertyGroup->m_description = "ScriptEvents used by the conversation editor as conditonals.";

        //AtomToolsFramework::Creat
        return propertyGroup;
    }

} // namespace ConversationEditor
