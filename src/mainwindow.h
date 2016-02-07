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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QMainWindow>
#include <QGraphicsView>
#include <QPlainTextEdit>
#include <QSpinBox>
#include "commandrunner.h"
#include "helpdialog.h"
#include "preferencesdialog.h"
#include "turtlegraphicsscene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void runCommand();
    void handleError(const QString& message);
    void commandUpdate();
    void commandFinished();
    void pauseCommand();
    void resumeCommand();
    void haltCommand();

private:

    Ui::MainWindow *ui;

    QPlainTextEdit* m_cmdEdit;
    QGraphicsView* m_view;
    QPushButton* m_runButton;
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    QPushButton* m_haltButton;

    TurtleGraphicsScene m_scene;
    CommandRunner m_cmds;

    PreferencesDialog* m_prefsDialog;
    HelpDialog* m_helpDialog;
};

#endif // MAINWINDOW_H
