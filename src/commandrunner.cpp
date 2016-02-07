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
#include "commandrunner.h"
#include "basiccommands.h"
#include <QMutexLocker>

CommandRunner::CommandRunner(TurtleGraphicsScene* const scene) :
    m_state(luaL_newstate()),
    m_scene(scene)
{
    // Load all libraries except io and os.
    // These libraries are omitted for security.
    luaL_requiref(m_state, "_G",        &luaopen_base,      1);
    luaL_requiref(m_state, "coroutine", &luaopen_coroutine, 1);
    luaL_requiref(m_state, "math",      &luaopen_math,      1);
    luaL_requiref(m_state, "package",   &luaopen_package,   1);
    luaL_requiref(m_state, "string",    &luaopen_string,    1);
    luaL_requiref(m_state, "table",     &luaopen_table,     1);
    luaL_requiref(m_state, "utf8",      &luaopen_utf8,      1);
    lua_pop(m_state, 7);


    setupCommands(m_state, scene);
}

void CommandRunner::requestThreadStop()
{
    requestInterruption();
    m_scriptDataSema.release();
}

/**
 * @brief Executes a Lua script script.
 *
 * If an error occurs then the @c commandError signal is emitted.
 *
 * @param[in] command String containing the Lua code to execute.
 */
void CommandRunner::runCommand(const QString& command)
{
    {
        QMutexLocker lock(&m_scriptDataMutex);
        m_scriptData.push_back(command);
    }
    m_scriptDataSema.release();
}

/**
 * @brief Open and run a Lua script file.
 *
 * If an error occurs in either loading or running the script then the
 * @c commandError signal is emitted.
 *
 * @param filename The name of the file to load and run.
 */
void CommandRunner::runScriptFile(const QString& filename)
{
    QMutexLocker lock(&m_luaMutex);
    bool success = false;

    if (LUA_OK == luaL_loadfile(m_state, filename.toStdString().c_str()))
    {
        if (LUA_OK == lua_pcall(m_state, 0, 0, 0))
        {
            success = true;
        }
    }

    if (!success)
    {
        const char* errmsg = lua_tostring(m_state, -1);
        if (NULL != errmsg)
        {
            emit commandError(QString(errmsg));
        }
    }
}

void CommandRunner::run()
{
    QString scriptData;
    bool hasScriptData;

    while (!isInterruptionRequested())
    {
        m_scriptDataSema.acquire();

        if (isInterruptionRequested())
        {
            break;
        }

        // Check if there's any scripts in the queue
        {
            QMutexLocker lock(&m_scriptDataMutex);
            if (m_scriptData.size() > 0)
            {
                scriptData = m_scriptData.front();
                m_scriptData.pop_front();
                hasScriptData = true;
            }
            else
            {
                hasScriptData = false;
            }
        }

        // Execute the lua script (if any)
        if (hasScriptData)
        {
            bool success = false;

            QMutexLocker lock(&m_luaMutex);
            if (LUA_OK == luaL_loadstring(m_state, scriptData.toStdString().c_str()))
            {
                if (LUA_OK == lua_pcall(m_state, 0, 0, 0))
                {
                    success = true;
                }
            }

            if (success)
            {
                emit commandFinished();
            }
            else
            {
                const char* errmsg = lua_tostring(m_state, -1);
                if (NULL != errmsg)
                {
                    emit commandError(QString(errmsg));
                }
            }
        }
    }
}
