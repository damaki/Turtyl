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
#include "settings.h"
#include "scriptrunner.h"
#include "aboutdialog.h"
#include "preferencesdialog.h"
#include "canvassaveoptionsdialog.h"
#include "turtlecanvasgraphicsitem.h"

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
    void runScript();

    void showScriptError(const QString& message);
    void showScriptOutput();
    void scriptFinished(bool hasErrors);

    void pauseScript();
    void resumeScript();
    void haltScript();

    void resizeGraphicsScene();

    void showErrors();
    void showScriptOutputs();

    void saveCanvas();

    void saveScript();
    void loadScript();

    void loadPreferences();
    void savePreferences();
    void applyPreferences();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* m_scene;
    TurtleCanvasGraphicsItem* m_turtleGraphics;
    ScriptRunner m_cmds;

    PreferencesDialog* m_prefsDialog;
    AboutDialog* m_aboutDialog;
    CanvasSaveOptionsDialog* m_canvasSaveOptionsDialog;

    Settings m_settings;
};

#endif // MAINWINDOW_H
