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

PreferencesDialog::PreferencesDialog(QWidget* const parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
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
