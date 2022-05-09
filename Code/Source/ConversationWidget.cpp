
#include <AzCore/Utils/Utils.h>

#include <QLabel>
#include <QVBoxLayout>

#include <ConversationWidget.h>
#include <ConversationEditor/ui_ConversationEditorWidget.h>

namespace Conversation
{
    ConversationWidget::ConversationWidget(QWidget* parent)
        : QWidget(parent)
    {
        m_editorWidgetUi = AZStd::make_unique<Ui::ConversationEditorWidget>();
        m_editorWidgetUi->setupUi(this);

    }
}

#include <moc_ConversationWidget.cpp>
