
#pragma once

#include <AtomToolsFramework/SettingsDialog/SettingsDialog.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/UserSettings/UserSettings.h>
#include <ScriptEvents/ScriptEventsAssetRef.h>

namespace ConversationEditor
{
    constexpr const AZ::Crc32 ConversationEditorSettingsId = AZ_CRC_CE("ConversationEditorSettings");

    class ConversationEditorSettings : public AZ::UserSettings
    {
    public:
        AZ_RTTI(ConversationEditorSettings, "{37C152C3-AF51-4D3A-9E9E-97DABC751107}", AZ::UserSettings);
        AZ_CLASS_ALLOCATOR(ConversationEditorSettings, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        ConversationEditorSettings();
        ~ConversationEditorSettings() override = default;

        const ScriptEvents::ScriptEventsAssetRef GetScriptEvents() const
        {
            return m_editorScriptEvents;
        }

    public:
        int m_enableDebug = false;
        ScriptEvents::ScriptEventsAssetRef m_editorScriptEvents;
    };

    class SettingsDialog : public AtomToolsFramework::SettingsDialog
    {
    };

    AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> CreateSettingsPropertyGroup();

} // namespace ConversationEditor
