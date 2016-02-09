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
#include "commandrunner.h"
#include "basiccommands.h"
#include <QMutexLocker>
#include <cassert>

ScriptRunner::ScriptRunner(TurtleGraphicsCanvasItem* const graphicsWidget) :
    m_state(luaL_newstate()),
    m_graphicsWidget(graphicsWidget),
    m_scriptDataSema(),
    m_scriptDataMutex(),
    m_scriptData(),
    m_pauseCond(),
    m_pauseMutex(),
    m_pause(false),
    m_haltMutex(),
    m_halt(false)
{
    assert(NULL != m_state);
    assert(NULL != graphicsWidget);

    // Load all libraries except io, os, and debug.
    // These libraries are omitted for security.
    luaL_requiref(m_state, "_G",        &luaopen_base,      1);
    luaL_requiref(m_state, "coroutine", &luaopen_coroutine, 1);
    luaL_requiref(m_state, "math",      &luaopen_math,      1);
    luaL_requiref(m_state, "package",   &luaopen_package,   1);
    luaL_requiref(m_state, "string",    &luaopen_string,    1);
    luaL_requiref(m_state, "table",     &luaopen_table,     1);
    luaL_requiref(m_state, "utf8",      &luaopen_utf8,      1);
    lua_pop(m_state, 7);


    setupCommands(m_state, this);
}

TurtleGraphicsCanvasItem* ScriptRunner::graphicsWidget() const
{
    return m_graphicsWidget;
}

/**
 * @brief Send a request to stop the thread.
 *
 * If any commands are currently being executed by the thread then the current command
 * is halted.
 */
void ScriptRunner::requestThreadStop()
{
    requestInterruption();
    haltScript();
    m_scriptDataSema.release(); // wake up the thread (if it's sleeping)
}

/**
 * @brief Send a request to pause the execution of a currently running command.
 */
void ScriptRunner::pauseScript()
{
    QMutexLocker lock(&m_pauseMutex);
    m_pause = true;
}

/**
 * @brief Send a request to resume the execution of a previously paused command.
 */
void ScriptRunner::resumeScript()
{
    QMutexLocker lock(&m_pauseMutex);
    if (m_pause)
    {
        m_pause = false;
        m_pauseCond.wakeAll();
    }
}

/**
 * @brief Send a request to halt/abort the execution of the current command(s).
 */
void ScriptRunner::haltScript()
{
    {
        QMutexLocker lock(&m_haltMutex);
        m_halt = true;
    }

    // The command might currently be paused.
    resumeScript();
}

/**
 * @brief Executes a Lua script script.
 *
 * If an error occurs then the @c commandError signal is emitted.
 *
 * @param[in] command String containing the Lua code to execute.
 */
void ScriptRunner::runCommand(const QString& command)
{
    {
        QMutexLocker lock(&m_pauseMutex);
        m_pause = false;
    }

    {
        QMutexLocker lock(&m_haltMutex);
        m_halt = false;
    }

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
void ScriptRunner::runScriptFile(const QString& filename)
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

    if (success)
    {
        emit scriptFinished(false);
    }
    else
    {
        const char* errmsg = lua_tostring(m_state, -1);
        if (NULL != errmsg)
        {
            emit scriptError(QString(errmsg));
        }

        emit scriptFinished(true);
    }
}

/**
 * @brief Waits until the script is resumed.
 *
 * If @c requestCommandPause() is called then this method will block until
 * @c requestCommandResume() is called by another thread.
 *
 * @see requestCommandPause()
 * @see requestCommandResume()
 */
void ScriptRunner::checkPause()
{
    QMutexLocker lock(&m_pauseMutex);
    while (m_pause)
    {
        m_pauseCond.wait(&m_pauseMutex);
    }
}

/**
 * @brief Check for a request to halt/abort the current execution of the script.
 *
 * @see requestCommandHalt()
 * @return Returns @c true if the script should halt. @c false otherwise.
 */
bool ScriptRunner::haltRequested() const
{
    QMutexLocker lock(&m_haltMutex);
    return m_halt;
}

void ScriptRunner::printMessage(const QString& message)
{
    emit scriptMessage(message);
}

void ScriptRunner::run()
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
                emit scriptFinished(false);
            }
            else
            {
                const char* errmsg = lua_tostring(m_state, -1);
                if (NULL != errmsg)
                {
                    emit scriptError(QString(errmsg));
                }

                emit scriptFinished(true);
            }
        }
    }
}
