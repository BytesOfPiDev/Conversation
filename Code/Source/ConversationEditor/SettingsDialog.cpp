#include <ConversationEditor/SettingsDialog.h>
#include <ScriptEvents/ScriptEventsAssetRef.h>

namespace ConversationEditor
{
    ConversationSettingsDialog::ConversationSettingsDialog(ConversationEditorSettings* editorSettings, QWidget* parent)
        : QDialog(parent)
        , m_editorSettings(editorSettings)
    {
        m_propertyEditor = aznew AzToolsFramework::ReflectedPropertyEditor(this);

        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

        m_propertyEditor->Setup(serializeContext, nullptr, false);
        m_propertyEditor->AddInstance(m_editorSettings);
        m_propertyEditor->InvalidateAll();
    }

    AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> ConversationSettingsDialog::CreateGroup()
    {
        AtomToolsFramework::DynamicProperty eventProperty;
        eventProperty.SetValue(AZStd::any(AZ::Data::Asset<ScriptEvents::ScriptEventsAsset>()));

        AtomToolsFramework::DynamicPropertyConfig eventPropertyConfig;
        eventPropertyConfig.m_dataType = AtomToolsFramework::DynamicPropertyType::Asset;
        eventPropertyConfig.m_defaultValue = 42;
        eventPropertyConfig.m_description = "An asset to use for conditionals";
        eventPropertyConfig.m_displayName = "Condition Script Event Asset";
        eventPropertyConfig.m_groupDisplayName = "Condition Script Events Group Display Name";
        eventPropertyConfig.m_groupName = "EventAssets";
        eventPropertyConfig.m_supportedAssetTypes = { ScriptEvents::ScriptEventsAsset::TYPEINFO_Uuid() };

        eventProperty.SetConfig(eventPropertyConfig);

        auto eventsGroup = AtomToolsFramework::CreateSettingsGroup("Event List", "Events for use as conditionals.", { eventProperty });

        return eventsGroup;
    } // namespace ConversationEditor
} // namespace ConversationEditor
