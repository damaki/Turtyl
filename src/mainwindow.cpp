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
#include <QScrollBar>
#include <cassert>
#include <cmath>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(new QGraphicsScene),
    m_turtleGraphics(new TurtleCanvasGraphicsItem),
    m_cmds(m_turtleGraphics),
    m_prefsDialog(new PreferencesDialog(m_turtleGraphics, this)),
    m_helpDialog(new HelpDialog(this))
{
    ui->setupUi(this);

    m_scene->addItem(m_turtleGraphics);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->centerOn(0.0, 0.0);

    ui->errorMessagesTextEdit->setTextColor(Qt::red);

    // These buttons are only enabled while a script is running.
    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    // Messages dock is hidden by default.
    ui->messagesDockWidget->hide();

    connect(m_turtleGraphics, SIGNAL(canvasResized()), this, SLOT(resizeGraphicsScene()));

    connect(ui->runButton,    SIGNAL(clicked()), this, SLOT(runScript()));
    connect(ui->haltButton,   SIGNAL(clicked()), this, SLOT(haltScript()));
    connect(ui->pauseButton,  SIGNAL(clicked()), this, SLOT(pauseScript()));
    connect(ui->resumeButton, SIGNAL(clicked()), this, SLOT(resumeScript()));

    connect(ui->action_Preferences, SIGNAL(triggered()), m_prefsDialog, SLOT(show()));
    connect(ui->action_Preferences, SIGNAL(triggered()), m_prefsDialog, SLOT(loadPreferences()));
    connect(ui->action_About,       SIGNAL(triggered()), m_helpDialog,  SLOT(show()));

    connect(ui->action_Errors, SIGNAL(triggered(bool)), this, SLOT(showErrors()));
    connect(ui->action_Script_Output, SIGNAL(triggered(bool)),
            this, SLOT(showScriptOutputs()));

    connect(&m_cmds, SIGNAL(scriptError(QString)),
            this,    SLOT(showScriptError(QString)),
            Qt::QueuedConnection);

    connect(&m_cmds, SIGNAL(scriptMessage(QString)),
            this,    SLOT(showScriptOutput(QString)),
            Qt::QueuedConnection);

    m_cmds.start();

    //TODO: Make startup scripts configurable.
    m_cmds.runScriptFile("print.lua");
    m_cmds.runScriptFile("turtle.lua");

    // Don't connect this until all the startup scripts have run
    // to prevent the error messages box from being cleared by successful scripts.
    connect(&m_cmds, SIGNAL(scriptFinished(bool)),
            this,    SLOT(scriptFinished(bool)),
            Qt::QueuedConnection);
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

void MainWindow::runScript()
{
    ui->errorMessagesTextEdit->clear();

    m_cmds.runScript(ui->cmdEdit->document()->toPlainText());

    ui->runButton->setEnabled(false);
    ui->haltButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);
}

void MainWindow::showScriptError(const QString& message)
{
    ui->errorMessagesTextEdit->append(message);

    if (m_prefsDialog->autoShowScriptErrors())
    {
        ui->messagesDockWidget->show();
        ui->messagesTabWidget->setCurrentWidget(ui->errorsTab);
    }
}

void MainWindow::showScriptOutput(const QString& message)
{
    ui->scriptMessagesTextEdit->appendPlainText(message);

    if (m_prefsDialog->autoShowScriptOutput())
    {
        ui->messagesDockWidget->show();
        ui->messagesTabWidget->setCurrentWidget(ui->outputTab);
    }
}

/**
 * @brief Called when a command finishes executing.
 *
 * This method updates the UI's buttons to allow another command to be executed.
 */
void MainWindow::scriptFinished(bool hasErrors)
{
    ui->runButton->setEnabled(true);
    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    if (!hasErrors)
    {
        ui->errorMessagesTextEdit->clear();
    }
}

/**
 * @brief Pauses the currently executing command(s)
 */
void MainWindow::pauseScript()
{
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(true);

    m_cmds.pauseScript();
}

/**
 * @brief Resumes a previously paused command.
 */
void MainWindow::resumeScript()
{
    ui->pauseButton->setEnabled(true);
    ui->resumeButton->setEnabled(false);

    m_cmds.resumeScript();
}

/**
 * @brief Halts/stops/aborts the currently running command.
 */
void MainWindow::haltScript()
{
    ui->runButton->setEnabled(false);
    ui->haltButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->resumeButton->setEnabled(false);

    m_cmds.haltScript();
}

/**
 * @brief Resets the QGraphicsScene to perfectly fit the turtle graphic's bounding box.
 *
 * This avoids unused space around the drawing canvas when the canvas size is reduced.
 */
void MainWindow::resizeGraphicsScene()
{
    m_scene->setSceneRect(m_turtleGraphics->boundingRect().translated(m_turtleGraphics->pos()));
}

void MainWindow::showErrors()
{
    ui->messagesDockWidget->show();
    ui->messagesTabWidget->setCurrentWidget(ui->errorsTab);
}

void MainWindow::showScriptOutputs()
{
    ui->messagesDockWidget->show();
    ui->messagesTabWidget->setCurrentWidget(ui->outputTab);
}
