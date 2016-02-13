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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

/**
 * @brief Handles persistent application settings.
 *
 * This class is a wrapper around QSettings to manage various persistent
 * settings, such as a list of scripts to be executed at startup.
 */
class Settings
{
public:
    struct Preferences
    {
        // Graphics
        int canvasWidth;
        int canvasHeight;
        bool antialiased;

        // Messages
        bool autoShowScriptErrors;
        bool autoShowScriptOutput;
    };

    Settings(const QString& filename);

    Preferences preferences() const;
    void setPreferences(const Preferences& prefs);

    QList<QString> startupScripts() const;
    void setStartupScripts(const QList<QString>& scripts);

    QList<QString> requirePaths() const;
    void setRequirePaths(const QList<QString>& paths);

private:
    mutable QSettings m_settings;
};

#endif // SETTINGS_H
