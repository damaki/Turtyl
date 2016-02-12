/************************************************************************
 * Copyright (c) 2016 Daniel King
 * Turtle graphics with Lua
 *
 * This file is part of Turtyl.
 *
 * Turtyl is free software: you can redistribute it and/or modify
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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "lua.hpp"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Make background transparent
    ui->descriptionTextEdit->viewport()->setAutoFillBackground(false);
    ui->titleTextBrowser->viewport()->setAutoFillBackground(false);

    // Fill in details in the title
    QString title = ui->titleTextBrowser->document()->toHtml();
    fillProgramDetails(title);
    ui->titleTextBrowser->document()->setHtml(title);

    // Fill in information in the description
    QString description = ui->descriptionTextEdit->document()->toHtml();
    fillProgramDetails(description);
    ui->descriptionTextEdit->document()->setHtml(description);

    // License boxes are hidden by default
    ui->licenseTextEdit->setVisible(false);
    ui->qtDescriptionTextEdit->setVisible(false);

    connect(ui->licenseButton, SIGNAL(clicked(bool)),
            this, SLOT(licenseClicked(bool)));
    connect(ui->qtButton, SIGNAL(clicked(bool)),
            this, SLOT(aboutQtClicked(bool)));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::licenseClicked(bool checked)
{
    if (checked)
    {
        ui->descriptionTextEdit->hide();
        ui->qtDescriptionTextEdit->hide();
        ui->licenseTextEdit->show();

        ui->qtButton->setChecked(false);
    }
    else
    {
        ui->qtDescriptionTextEdit->hide();
        ui->licenseTextEdit->hide();
        ui->descriptionTextEdit->show();
    }
}

void AboutDialog::aboutQtClicked(bool checked)
{
    if (checked)
    {
        ui->descriptionTextEdit->hide();
        ui->licenseTextEdit->hide();
        ui->qtDescriptionTextEdit->show();

        ui->licenseButton->setChecked(false);
    }
    else
    {
        ui->licenseTextEdit->hide();
        ui->qtDescriptionTextEdit->hide();
        ui->descriptionTextEdit->show();
    }
}

void AboutDialog::fillProgramDetails(QString& string)
{
    string.replace("$APP_VERSION", APP_VERSION)
            .replace("$QT_VERSION", QT_VERSION_STR)
            .replace("$LUA_COPYRIGHT", LUA_COPYRIGHT)
            .replace("$BUILD_DATE", __DATE__)
            .replace("$BUILD_TIME", __TIME__);
}
