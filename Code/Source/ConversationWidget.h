
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <QWidget>

#endif

namespace Ui
{
    class ConversationEditorWidget;
}

namespace Conversation
{


    class ConversationWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit ConversationWidget(QWidget* parent = nullptr);

    private:
        AZStd::unique_ptr<Ui::ConversationEditorWidget> m_editorWidgetUi;
    };
} // namespace Conversation
