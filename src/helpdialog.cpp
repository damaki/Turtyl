/************************************************************************
 * Copyright (c) 2016 Daniel King
 * Turtle graphics with Lua
 *
 * This file is part of Tartaruga.
 *
 * Tartaruga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/
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
