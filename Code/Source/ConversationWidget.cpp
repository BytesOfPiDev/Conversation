
#include <AzCore/Utils/Utils.h>

#include <QLabel>
#include <QVBoxLayout>

#include <ConversationWidget.h>
#include <ConversationEditor/ui_DialogueEditorWidget.h>

namespace Conversation
{
    ConversationWidget::ConversationWidget(QWidget* parent)
        : QWidget(parent)
    {
        m_editorWidgetUi = AZStd::make_unique<Ui::DialogueEditorWidget>();
        m_editorWidgetUi->setupUi(this);

    }
}

#include <moc_ConversationWidget.cpp>
