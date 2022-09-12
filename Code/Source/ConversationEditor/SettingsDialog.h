#pragma once

#if !defined(Q_MOC_RUN) // Required, or else AUTOMOC uses AZ namespace.
#include <AtomToolsFramework/SettingsDialog/SettingsDialog.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>

#include <ConversationEditor/Settings.h>
#endif

namespace ConversationEditor
{
    class ConversationSettingsDialog : public QDialog
    {
        Q_OBJECT

    public:
        ConversationSettingsDialog(ConversationEditorSettings* editorSettings, QWidget* parent = nullptr);
        ~ConversationSettingsDialog() override = default;

        static AZStd::shared_ptr<AtomToolsFramework::DynamicPropertyGroup> CreateGroup();

    private:
        ConversationEditorSettings* m_editorSettings;
        AzToolsFramework::ReflectedPropertyEditor* m_propertyEditor;
    };
} // namespace ConversationEditor
