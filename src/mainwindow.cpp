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

    m_cmdEdit      = findChild<QPlainTextEdit*>("cmdEdit");
    m_view         = findChild<QGraphicsView*>("graphicsView");
    m_runButton    = findChild<QPushButton*>("runButton");
    m_haltButton   = findChild<QPushButton*>("haltButton");
    m_pauseButton  = findChild<QPushButton*>("pauseButton");
    m_resumeButton = findChild<QPushButton*>("resumeButton");

    QAction* prefsAction = findChild<QAction*>("action_Preferences");
    QAction* helpAction  = findChild<QAction*>("action_About");

    if (NULL != m_view)
    {
        m_scene.setView(m_view);
        m_view->centerOn(0.0, 0.0);
        m_view->show();
    }

    if (m_runButton != NULL)
    {
        connect(m_runButton, SIGNAL(clicked()), this, SLOT(runCommand()));
    }

    if (NULL != m_haltButton)
    {
        m_haltButton->setEnabled(false);
        connect(m_haltButton, SIGNAL(clicked()), this, SLOT(haltCommand()));
    }

    if (NULL != m_pauseButton)
    {
        m_pauseButton->setEnabled(false);
        connect(m_pauseButton, SIGNAL(clicked()), this, SLOT(pauseCommand()));
    }

    if (NULL != m_resumeButton)
    {
        m_resumeButton->setEnabled(false);
        connect(m_resumeButton, SIGNAL(clicked()), this, SLOT(resumeCommand()));
    }

    if (NULL != prefsAction)
    {
        connect(prefsAction, SIGNAL(triggered()), m_prefsDialog, SLOT(show()));
        connect(prefsAction, SIGNAL(triggered()), m_prefsDialog, SLOT(loadPreferences()));
    }

    if (NULL != helpAction)
    {
        connect(helpAction, SIGNAL(triggered()), m_helpDialog, SLOT(show()));
    }

    connect(&m_cmds, SIGNAL(commandError(QString)),
            this,    SLOT(handleError(QString)),
            Qt::QueuedConnection);

    connect(&m_cmds, SIGNAL(commandFinished()),
            this,    SLOT(commandFinished()),
            Qt::QueuedConnection);

    m_cmds.setView(m_view);
    m_cmds.start();

    //TODO: Make startup scripts configurable.
    m_cmds.runScriptFile("turtle.lua");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    // Stop the command thread
    m_cmds.requestThreadStop();
    m_cmds.wait();
}

void MainWindow::runCommand()
{
    if (NULL != m_cmdEdit)
    {
        m_cmds.runCommand(m_cmdEdit->document()->toPlainText());
        m_scene.updateScene();

        if (NULL != m_runButton)
        {
            m_runButton->setEnabled(false);
        }
        if (NULL != m_haltButton)
        {
            m_haltButton->setEnabled(true);
        }
        if (NULL != m_pauseButton)
        {
            m_pauseButton->setEnabled(true);
        }
    }
}

void MainWindow::handleError(const QString& message)
{
    //TODO: Display error messages in the GUI
    std::cerr << message.toStdString() << std::endl;
}

void MainWindow::commandUpdate()
{
    m_scene.updateScene();
}

void MainWindow::commandFinished()
{
    m_scene.updateScene();

    if (NULL != m_runButton)
    {
        m_runButton->setEnabled(true);
    }
    if (NULL != m_haltButton)
    {
        m_haltButton->setEnabled(false);
    }
    if (NULL != m_pauseButton)
    {
        m_pauseButton->setEnabled(false);
    }
    if (NULL != m_resumeButton)
    {
        m_resumeButton->setEnabled(false);
    }
}

void MainWindow::pauseCommand()
{
    if (NULL != m_pauseButton)
    {
        m_pauseButton->setEnabled(false);
    }
    if (NULL != m_resumeButton)
    {
        m_resumeButton->setEnabled(true);
    }

    m_cmds.requestCommandPause();
}

void MainWindow::resumeCommand()
{
    if (NULL != m_pauseButton)
    {
        m_pauseButton->setEnabled(true);
    }
    if (NULL != m_resumeButton)
    {
        m_resumeButton->setEnabled(false);
    }

    m_cmds.requestCommandResume();
}

void MainWindow::haltCommand()
{
    if (NULL != m_runButton)
    {
        m_runButton->setEnabled(false);
    }
    if (NULL != m_haltButton)
    {
        m_haltButton->setEnabled(false);
    }
    if (NULL != m_pauseButton)
    {
        m_pauseButton->setEnabled(false);
    }
    if (NULL != m_resumeButton)
    {
        m_resumeButton->setEnabled(false);
    }

    m_cmds.requestCommandHalt();
}
