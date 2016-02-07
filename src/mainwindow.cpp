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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cassert>
#include <cmath>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cmdEdit(NULL),
    m_view(NULL),
    m_scene(),
    m_cmds(&m_scene),
    m_prefsDialog(new PreferencesDialog(&m_scene, this)),
    m_helpDialog(new HelpDialog(this))
{
    ui->setupUi(this);

    m_cmdEdit = findChild<QPlainTextEdit*>("cmdEdit");
    m_view    = findChild<QGraphicsView*>("graphicsView");

    if (NULL != m_view)
    {
        m_view->setScene(&m_scene.scene());

        // Setup an initial scene rect. Otherwise, some drawings aren't centered properly.
        m_view->setSceneRect(QRectF(-1.0, -1.0, 2.0, 2.0));

        m_view->centerOn(0.0, 0.0);
        m_view->show();
    }

    // Connect the "Run" button to runCommand()
    QPushButton* runButton = findChild<QPushButton*>("runButton");
    if (runButton != NULL)
    {
        connect(runButton, SIGNAL(clicked()), this, SLOT(runCommand()));
    }

    QAction* prefsAction = findChild<QAction*>("action_Preferences");
    if (NULL != prefsAction)
    {
        connect(prefsAction, SIGNAL(triggered()), m_prefsDialog, SLOT(show()));
        connect(prefsAction, SIGNAL(triggered()), m_prefsDialog, SLOT(loadPreferences()));
    }

    QAction* helpAction = findChild<QAction*>("action_About");
    if (NULL != helpAction)
    {
        connect(helpAction, SIGNAL(triggered()), m_helpDialog, SLOT(show()));
    }

    connect(&m_cmds, SIGNAL(commandError(QString)), this, SLOT(handleError(QString)));

    //TODO: Make startup scripts configurable.
    m_cmds.runScriptFile("turtle.lua");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runCommand()
{
    if (NULL != m_cmdEdit)
    {
        m_cmds.runCommand(m_cmdEdit->document()->toPlainText());
        m_scene.updateScene();
    }
}

void MainWindow::handleError(const QString& message)
{
    //TODO: Display error messages in the GUI
    std::cerr << message.toStdString() << std::endl;
}

