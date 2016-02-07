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

PreferencesDialog::PreferencesDialog(TurtleGraphicsScene* const scene,
                                     QWidget* const parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    m_scene(scene)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(applyPreferences()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::loadPreferences()
{
    if (NULL != m_scene)
    {
        ui->antialiasingCheckBox->setChecked(m_scene->antialiasingEnabled());
        ui->canvasSizeSpinBox->setValue(m_scene->canvasSize());
    }
}

void PreferencesDialog::applyPreferences()
{
    if (NULL != m_scene)
    {
        m_scene->setAntialiasing(ui->antialiasingCheckBox->checkState() == Qt::Checked);
        m_scene->setCanvasSize(ui->canvasSizeSpinBox->value());

        m_scene->updateScene();
    }
}
