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
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>

namespace Ui
{
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget* parent = nullptr);
    ~PreferencesDialog();

    QSize canvasSize() const;
    void setCanvasSize(QSize size);

    bool antialias() const;
    void setAntialias(bool on);

    bool autoShowScriptErrors() const;
    void setAutoShowScriptErrors(bool on);

    bool autoShowScriptOutput() const;
    void setAutoShowScriptOutput(bool on);

    QList<QString> startupScripts() const;
    void setStartupScripts(const QList<QString>& scripts);

    QList<QString> requirePaths() const;
    void setRequirePaths(const QList<QString>& paths);

private slots:
    void addStartupScript();
    void addStartupScriptFile();
    void removeStartupScripts();

    void addRequirePath();
    void removeRequirePaths();

private:
    Ui::PreferencesDialog* ui;
};

#endif // PREFERENCESDIALOG_H
