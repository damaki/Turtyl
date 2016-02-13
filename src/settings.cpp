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
#include "settings.h"

Settings::Settings(const QString& filename) :
    m_settings(filename, QSettings::IniFormat)
{
}

/**
 * @brief Get the current preferences settings from persistent storage.
 *
 * @return Various preference settings.
 */
Settings::Preferences Settings::preferences() const
{
    Preferences prefs;

    m_settings.beginGroup("preferences");

    m_settings.beginGroup("canvas");
    prefs.canvasWidth  = m_settings.value("width", 2048).toInt();
    prefs.canvasHeight = m_settings.value("height", 2048).toInt();
    prefs.antialiased  = m_settings.value("antialis", false).toBool();
    m_settings.endGroup();

    m_settings.beginGroup("messages");
    prefs.autoShowScriptErrors = m_settings.value("autoShowScriptErrors", true).toBool();
    prefs.autoShowScriptOutput = m_settings.value("autoShowScriptOutput", true).toBool();
    m_settings.endGroup();

    m_settings.endGroup();

    return prefs;
}

/**
 * @brief Sets perferences to be saved in persistent storage.
 *
 * @param prefs The preferences to save.
 */
void Settings::setPreferences(const Preferences& prefs)
{
    m_settings.beginGroup("preferences");

    m_settings.beginGroup("canvas");
    m_settings.setValue("width", prefs.canvasWidth);
    m_settings.setValue("height", prefs.canvasHeight);
    m_settings.setValue("antialis", prefs.antialiased);
    m_settings.endGroup();

    m_settings.beginGroup("messages");
    m_settings.setValue("autoShowScriptErrors", prefs.autoShowScriptErrors);
    m_settings.setValue("autoShowScriptOutput", prefs.autoShowScriptOutput);
    m_settings.endGroup();

    m_settings.endGroup();
}

QList<QString> Settings::startupScripts() const
{
    QList<QString> scripts;
    int size = m_settings.beginReadArray("startup");

    for (int i = 0; i < size; i++)
    {
        m_settings.setArrayIndex(i);
        scripts.append(m_settings.value("script").toString());
    }

    m_settings.endArray();

    return scripts;
}

void Settings::setStartupScripts(const QList<QString>& scripts)
{
    m_settings.beginWriteArray("startup", scripts.size());

    int i = 0;
    for (const QString& str : scripts)
    {
        m_settings.setArrayIndex(i);
        m_settings.setValue("script", str);
        ++i;
    }

    m_settings.endArray();
}

QList<QString> Settings::requirePaths() const
{
    QList<QString> paths;
    int size = m_settings.beginReadArray("require");

    for (int i = 0; i < size; i++)
    {
        m_settings.setArrayIndex(i);
        paths.append(m_settings.value("path").toString());
    }

    m_settings.endArray();

    return paths;
}

void Settings::setRequirePaths(const QList<QString>& paths)
{
    m_settings.beginWriteArray("require", paths.size());

    int i = 0;
    for (const QString& str : paths)
    {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", str);
        ++i;
    }

    m_settings.endArray();
}
