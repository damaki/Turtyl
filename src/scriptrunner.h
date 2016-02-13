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
 * produce an error message via the @c scriptError(QString) signal.
 *
 * @section Script Messages
 *
 * Scripts can call the @c _ui.print() message to print strings. When
 * a script calls @c _ui.print() the scriptMessageReceived() signal
 * is emitted.
 *
 * After the @c scriptMessageReceived() signal has been emitted
 * pendingScriptMessage() must be called to read the message.
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

    void setRequirePath(const QString& path);
    void addRequirePath(QString path);

    void runScript(const QString& script);

    void runScriptFile(const QString& filename);

    QString pendingScriptMessage();
    void clearPendingScriptMessage();

signals:
    void scriptFinished(bool hasErrors);

    /**
     * @brief This signal is emitted when a script encounters an error.
     *
     *
     *
     * @param message A string containing a displayable error message.
     */
    void scriptError(const QString& message);

    /**
     * @brief This signal is emitted when the script has printed a message.
     *
     * The message is stored internally in a queue of all pending messages.
     *
     * The messages can be read by calling pendingScriptMessages().
     */
    void scriptMessageReceived();

protected:
    virtual void run();

private:
    bool haltRequested() const;
    void emitMessage(const QString& message);

    void openRestrictedBaseModule();
    void openRestrictedOsModule();

    void setupCommands(lua_State* state);

    void debugHook(lua_State* state);

    static int drawLine(lua_State* state);
    static int drawArc(lua_State* state);
    static int clearScreen(lua_State* state);
    static int setBackgroundColor(lua_State* state);
    static int getBackgroundColor(lua_State* state);
    static int setTurtle(lua_State* state);
    static int getTurtle(lua_State* state);
    static int showTurtle(lua_State* state);
    static int hideTurtle(lua_State* state);
    static int turtleHidden(lua_State* state);
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

    QMutex m_scriptMessageMutex;
    QWaitCondition m_scriptMessageCond;
    QString m_scriptMessage;
    volatile bool m_scriptMessagePending;
};

#endif // COMMANDRUNNER_H
