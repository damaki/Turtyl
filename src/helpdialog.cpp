#include "src/helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    m_descriptionTextEdit = findChild<QTextEdit*>("descriptionTextEdit");

    if (NULL != m_descriptionTextEdit)
    {
        // Make background transparent
        m_descriptionTextEdit->viewport()->setAutoFillBackground(false);

        // Fill in information
        QString description = m_descriptionTextEdit->document()->toHtml();
        description.replace("$APP_VERSION", APP_VERSION)
                .replace("$QT_VERSION", QT_VERSION_STR)
                .replace("$BUILD_TIME", __DATE__ " " __TIME__);
        m_descriptionTextEdit->document()->setHtml(description);
    }
}

HelpDialog::~HelpDialog()
{
    delete ui;
}
