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
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QTextStream>
#include <cassert>
#include <cmath>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(new QGraphicsScene),
    m_turtleGraphics(new TurtleCanvasGraphicsItem),
    m_cmds(m_turtleGraphics),
    m_prefsDialog(new PreferencesDialog(this)),
    m_aboutDialog(new AboutDialog(this))
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

    connect(ui->action_Open_Script, SIGNAL(triggered()), this, SLOT(loadScript()));
    connect(ui->action_Save_Script, SIGNAL(triggered()), this, SLOT(saveScript()));
    connect(ui->action_Save_Canvas, SIGNAL(triggered()), this, SLOT(saveCanvas()));
    connect(ui->action_Preferences, SIGNAL(triggered()), m_prefsDialog, SLOT(show()));
    connect(ui->action_About,       SIGNAL(triggered()), m_aboutDialog,  SLOT(show()));

    connect(ui->action_Errors, SIGNAL(triggered(bool)), this, SLOT(showErrors()));
    connect(ui->action_Script_Output, SIGNAL(triggered(bool)),
            this, SLOT(showScriptOutputs()));

    connect(m_prefsDialog, SIGNAL(accepted()), this, SLOT(applyPreferences()));
    connect(m_prefsDialog, SIGNAL(rejected()), this, SLOT(restorePreferences()));

    connect(&m_cmds, SIGNAL(scriptError(QString)),
            this,    SLOT(showScriptError(QString)),
            Qt::QueuedConnection);

    connect(&m_cmds, SIGNAL(scriptMessage(QString)),
            this,    SLOT(showScriptOutput(QString)),
            Qt::QueuedConnection);

    restorePreferences();

    m_cmds.start();

    //TODO: Make startup scripts configurable.
    m_cmds.addRequirePath("scripts/?.lua");
    m_cmds.runScriptFile("print.lua");
    m_cmds.runScriptFile("turtle.lua");
    m_cmds.runScriptFile("stdlib.lua");

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

    m_cmds.runScript(ui->scriptTextEdit->document()->toPlainText());

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

/**
 * @brief Makes the "Errors" tab visible.
 */
void MainWindow::showErrors()
{
    ui->messagesDockWidget->show();
    ui->messagesTabWidget->setCurrentWidget(ui->errorsTab);
}

/**
 * @brief Makes the "Script Outputs" tab visible.
 */
void MainWindow::showScriptOutputs()
{
    ui->messagesDockWidget->show();
    ui->messagesTabWidget->setCurrentWidget(ui->outputTab);
}

/**
 * @brief Saves the current canvas to an image file.
 *
 * A QFileDialog is shown to the user to request the file name.
 */
void MainWindow::saveCanvas()
{
    // Build a list of supported image formats to be used by the save dialog
    // E.g. "Images (*.bmp *.jpg)"
    QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();
    QString filter = tr("Images") + " (";
    bool first = true;
    for (QByteArray fmt : imageFormats)
    {
        if (!first)
        {
            // Separate each item with a space character
            filter.append(' ');
        }
        first = false;

        filter.append("*.");
        filter.append(fmt);
    }
    filter += ")";


    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setNameFilter(filter);
    fileDialog.setWindowTitle(tr("Save Canvas"));

    if (fileDialog.exec() != 0)
    {
        const QImage canvasImage(m_turtleGraphics->toImage(true));

        for (QString filename : fileDialog.selectedFiles())
        {
            QImageWriter writer(filename);

            if (!writer.write(canvasImage))
            {
                QString message = tr("Cannot write to file: ") + filename + '\n'
                                  + writer.errorString();
                QMessageBox::critical(this, tr("Save Error"), message);
            }
        }
    }
}

void MainWindow::saveScript()
{
    QStringList filters;
    filters << tr("Lua (*.lua)")
            << tr("Text (*.txt)")
            << tr("All Files (*)");

    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setNameFilters(filters);
    fileDialog.setWindowTitle(tr("Save Script"));

    if (fileDialog.exec() != 0)
    {
        const QString script = ui->scriptTextEdit->document()->toPlainText();

        for (QString filename : fileDialog.selectedFiles())
        {
            QFile file(filename);
            if (file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate))
            {
                QTextStream stream(&file);
                stream << script;
                stream.flush();
                file.close();
            }
            else
            {
                QString message = QString("Cannot save file: %1\n%2")
                        .arg(filename)
                        .arg(file.errorString());
                QMessageBox::critical(this, tr("Save Error"), message);
            }
        }
    }
}

void MainWindow::loadScript()
{
    QStringList filters;
    filters << tr("Lua (*.lua)")
            << tr("Text (*.txt)")
            << tr("All Files (*)");

    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setNameFilters(filters);
    fileDialog.setWindowTitle(tr("Open Script"));

    if (fileDialog.exec() != 0)
    {
        ui->scriptTextEdit->clear();

        for (QString filename : fileDialog.selectedFiles())
        {
            QFile file(filename);
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream stream(&file);
                ui->scriptTextEdit->appendPlainText(stream.readAll());
                file.close();
            }
            else
            {
                QString message = QString("Cannot open file: %1\n%2")
                        .arg(filename)
                        .arg(file.errorString());
                QMessageBox::critical(this, tr("Open Error"), message);
            }
        }
    }
}

void MainWindow::restorePreferences()
{
    m_prefsDialog->setAntialias(m_turtleGraphics->antialiased());
    m_prefsDialog->setCanvasSize(m_turtleGraphics->size().width());
}

void MainWindow::applyPreferences()
{
    m_turtleGraphics->setAntialiased(m_prefsDialog->antialias());
    m_turtleGraphics->resize(QSize(m_prefsDialog->canvasSize(),
                                   m_prefsDialog->canvasSize()));
}
