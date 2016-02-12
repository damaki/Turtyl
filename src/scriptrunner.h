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
#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <QThread>
#include <QSemaphore>
#include <QQueue>
#include <QWaitCondition>
#include "turtlecanvasgraphicsitem.h"
#include "lua.hpp"

/**
 * @brief Runs commands from Lua scripts.
 *
 * Errors produced when either loading/compiling or running a script
 * produce an error message via the @c commandError() signal.
 */
class ScriptRunner : public QThread
{
    Q_OBJECT

public:
    ScriptRunner(TurtleCanvasGraphicsItem* graphicsWidget);

    TurtleCanvasGraphicsItem* graphicsWidget() const;

    void requestThreadStop();

    void pauseScript();
    void resumeScript();
    void haltScript();

    void addRequirePath(QString path);

    void runScript(const QString& script);

    void runScriptFile(const QString& filename);

    bool haltRequested() const;
    void printMessage(const QString& message);

signals:
    void scriptFinished(bool hasErrors);
    void scriptError(const QString& message);

    void scriptMessage(const QString& message);

protected:
    virtual void run();

private:
    void setupCommands(lua_State* state);

    void debugHook(lua_State* state);

    static int drawLine(lua_State* state);
    static int drawArc(lua_State* state);
    static int clearScreen(lua_State* state);
    static int setBackgroundColor(lua_State* state);
    static int getBackgroundColor(lua_State* state);
    static int printMessage(lua_State* state);

    static void debugHookEntry(lua_State* state, lua_Debug* );


    lua_State* m_state;
    TurtleCanvasGraphicsItem* m_graphicsWidget;

    mutable QMutex m_luaMutex; // locked while a script is running

    QSemaphore m_scriptsQueueSema;
    mutable QMutex m_scriptsQueueMutex;
    QQueue<QString> m_scriptsQueue;

    QWaitCondition m_pauseCond;
    mutable QMutex m_pauseMutex;
    volatile bool m_pause;

    mutable QMutex m_haltMutex;
    volatile bool m_halt;
};

#endif // COMMANDRUNNER_H
