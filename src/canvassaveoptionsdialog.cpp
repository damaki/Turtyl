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
#include "canvassaveoptionsdialog.h"
#include "ui_canvassaveoptionsdialog.h"

CanvasSaveOptionsDialog::CanvasSaveOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CanvasSaveOptionsDialog)
{
    ui->setupUi(this);
}

CanvasSaveOptionsDialog::~CanvasSaveOptionsDialog()
{
    delete ui;
}

bool CanvasSaveOptionsDialog::fitToUsedArea() const
{
    return ui->fitToDrawingCheckBox->checkState() == Qt::Checked;
}

void CanvasSaveOptionsDialog::setFitToUsedArea(bool on)
{
    ui->fitToDrawingCheckBox->setChecked(on);
}

bool CanvasSaveOptionsDialog::transparentBackground() const
{
    return ui->transparentBackgroundCheckBox->checkState() == Qt::Checked;
}

void CanvasSaveOptionsDialog::setTransparentBackground(bool on)
{
    ui->transparentBackgroundCheckBox->setChecked(on);
}
