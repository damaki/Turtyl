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
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget* const parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    connect(ui->addStartupScriptButton, SIGNAL(clicked()),
            this, SLOT(addStartupScript()));
    connect(ui->addStartupScriptFileButton, SIGNAL(clicked()),
            this, SLOT(addStartupScriptFile()));
    connect(ui->removeStartupScriptButton, SIGNAL(clicked()),
            this, SLOT(removeStartupScripts()));

    connect(ui->addRequirePathButton, SIGNAL(clicked()),
            this, SLOT(addRequirePath()));
    connect(ui->removeRequirePathsButton, SIGNAL(clicked()),
            this, SLOT(removeRequirePaths()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

QSize PreferencesDialog::canvasSize() const
{
    return QSize(ui->canvasWidthSpinBox->value(),
                 ui->canvasHeightSpinBox->value());
}

void PreferencesDialog::setCanvasSize(QSize size)
{
    ui->canvasWidthSpinBox->setValue(size.width());
    ui->canvasHeightSpinBox->setValue(size.height());
}

bool PreferencesDialog::antialias() const
{
    return ui->antialiasingCheckBox->checkState() == Qt::Checked;
}

void PreferencesDialog::setAntialias(bool on)
{
    ui->antialiasingCheckBox->setChecked(on);
}

bool PreferencesDialog::autoShowScriptErrors() const
{
    return ui->autoShowErrorsCheckBox->checkState() == Qt::Checked;
}

void PreferencesDialog::setAutoShowScriptErrors(bool on)
{
    ui->autoShowErrorsCheckBox->setChecked(on);
}

bool PreferencesDialog::autoShowScriptOutput() const
{
    return ui->autoShowOutputCheckBox->checkState() == Qt::Checked;
}

void PreferencesDialog::setAutoShowScriptOutput(bool on)
{
    ui->autoShowOutputCheckBox->setChecked(on);
}

QList<QString> PreferencesDialog::startupScripts() const
{
    QList<QString> scripts;

    const int size = ui->startupScriptListWidget->count();
    for (int i = 0; i < size; i++)
    {
        QListWidgetItem* item = ui->startupScriptListWidget->item(i);
        if (item != NULL)
        {
            scripts.append(item->text());
        }
    }

    return scripts;
}

void PreferencesDialog::setStartupScripts(const QList<QString>& scripts)
{
    ui->startupScriptListWidget->clear();
    for (const QString& str : scripts)
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(str);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->startupScriptListWidget->addItem(item);
    }
}

QList<QString> PreferencesDialog::requirePaths() const
{
    QList<QString> paths;

    const int size = ui->requirePathsListWidget->count();
    for (int i = 0; i < size; i++)
    {
        QListWidgetItem* item = ui->requirePathsListWidget->item(i);
        if (item != NULL)
        {
            paths.append(item->text());
        }
    }

    return paths;
}

void PreferencesDialog::setRequirePaths(const QList<QString>& paths)
{
    ui->requirePathsListWidget->clear();
    for (const QString& str : paths)
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(str);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->requirePathsListWidget->addItem(item);
    }
}

void PreferencesDialog::addStartupScript()
{
    QListWidgetItem* item = new QListWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->startupScriptListWidget->addItem(item);
    ui->startupScriptListWidget->editItem(item);
}

void PreferencesDialog::addStartupScriptFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setNameFilter("Lua (*.lua)");
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Select Startup Script"));

    if (fileDialog.exec())
    {
        for (const QString& filename : fileDialog.selectedFiles())
        {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(filename);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->startupScriptListWidget->addItem(item);
        }
    }
}

void PreferencesDialog::removeStartupScripts()
{
    // Remove the selected items
    for (QListWidgetItem* item : ui->startupScriptListWidget->selectedItems())
    {
        delete item;
    }
}

void PreferencesDialog::addRequirePath()
{
    QListWidgetItem* item = new QListWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->requirePathsListWidget->addItem(item);
    ui->requirePathsListWidget->editItem(item);
}

void PreferencesDialog::removeRequirePaths()
{
    // Remove the selected items
    for (QListWidgetItem* item : ui->requirePathsListWidget->selectedItems())
    {
        delete item;
    }
}
