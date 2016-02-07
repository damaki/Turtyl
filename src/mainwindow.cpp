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

static const int VIEW_UPDATE_DELAY = 33; // update every 33ms (~30 fps)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(),
    m_cmds(&m_scene),
    m_prefsDialog(new PreferencesDialog(&m_scene, this)),
    m_helpDialog(new HelpDialog(this)),
    m_timerId(0)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(&m_scene.scene());
    ui->graphicsView->centerOn(0.0, 0.0);
    ui->graphicsView->show();

    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    connect(ui->runButton,    SIGNAL(clicked()), this, SLOT(runCommand()));
    connect(ui->haltButton,   SIGNAL(clicked()), this, SLOT(haltCommand()));
    connect(ui->pauseButton,  SIGNAL(clicked()), this, SLOT(pauseCommand()));
    connect(ui->resumeButton, SIGNAL(clicked()), this, SLOT(resumeCommand()));

    connect(ui->action_Preferences, SIGNAL(triggered()), m_prefsDialog, SLOT(show()));
    connect(ui->action_Preferences, SIGNAL(triggered()), m_prefsDialog, SLOT(loadPreferences()));
    connect(ui->action_About,       SIGNAL(triggered()), m_helpDialog,  SLOT(show()));

    connect(&m_cmds, SIGNAL(commandError(QString)),
            this,    SLOT(handleError(QString)),
            Qt::QueuedConnection);

    connect(&m_cmds, SIGNAL(commandFinished()),
            this,    SLOT(commandFinished()),
            Qt::QueuedConnection);

    m_cmds.setView(ui->graphicsView);
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

void MainWindow::timerEvent(QTimerEvent *)
{
    m_scene.updateScene();
}

void MainWindow::runCommand()
{
    m_cmds.runCommand(ui->cmdEdit->document()->toPlainText());
    m_scene.updateScene();

    ui->runButton->setEnabled(false);
    ui->haltButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);

    m_timerId = startTimer(VIEW_UPDATE_DELAY);
}

void MainWindow::handleError(const QString& message)
{
    //TODO: Display error messages in the GUI
    std::cerr << message.toStdString() << std::endl;
}

void MainWindow::commandFinished()
{
    if (m_timerId != 0)
    {
        killTimer(m_timerId);
    }

    m_scene.updateScene();

    ui->runButton->setEnabled(true);
    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);
}

void MainWindow::pauseCommand()
{
    // Don't refresh the view while the user script is paused
    if (m_timerId != 0)
    {
        killTimer(m_timerId);
    }

    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(true);

    m_cmds.requestCommandPause();
}

void MainWindow::resumeCommand()
{
    m_timerId = startTimer(VIEW_UPDATE_DELAY);

    ui->pauseButton->setEnabled(true);
    ui->resumeButton->setEnabled(false);

    m_cmds.requestCommandResume();
}

void MainWindow::haltCommand()
{
    ui->runButton->setEnabled(false);
    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    m_cmds.requestCommandHalt();
}
