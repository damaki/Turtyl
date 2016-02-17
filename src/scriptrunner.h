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
 * @section Running scripts
 *
 * Arbitrary Lua scripts can be run by calling runScript() and passing
 * the Lua source code as the string argument. The script is executed
 * by the ScriptRunner thread to avoid blocking the caller. Therefore,
 * runScript() will return immediately, before the script has finished
 * running.
 *
 * When the script has finished executing the scriptFinished() signal
 * is emitted. The boolean parameter for scriptFinished() denotes
 * the termination condition for the script i.e. whether or not an
 * error occurred.
 *
 * @subsection Controlling Script Execution
 *
 * While a script is running it is possible to pause and resume execution, or
 * prematurely halt/stop/terminate the script execution.
 *
 * The pauseScript() method will pause a currently running script, i.e. the
 * script will be blocked until resumeScript() is called. No Lua instructions
 * are executed whilst the script is paused.
 *
 * @note When calling pauseScript() there may be a small delay (several
 * Lua instructions) until the script is actually paused.
 *
 * A running script can be stopped prematurely by calling haltScript().
 * Calling haltScript() will cause the script to terminate as soon as
 * possible, regardless of what the script is currently doing.
 *
 * @section Drawing
 *
 * The ScriptRunner registers several Lua functions that are accessible
 * by scripts. These functions are used to modify the canvas, such as
 * drawing lines. The target canvas is passed in the constructor of the
 * ScriptRunner.
 *
 * @section Script Messages
 *
 * Scripts can call the @c _ui.print() message to print strings. When
 * a script calls @c _ui.print() the scriptMessageReceived() signal
 * is emitted.
 *
 * After the @c scriptMessageReceived() signal has been emitted
 * pendingScriptMessage() must be called to read the message.
 *
 * @warning There @b must be a slot handler for scriptMessageReceived()
 * which calls @c pendingScriptMessage() or clearPendingScriptMessage().
 * Otherwise, the script may be blocked until it is explicitly halted.
 */
class ScriptRunner : public QThread
{
    Q_OBJECT

public:
    ScriptRunner(TurtleCanvasGraphicsItem* graphicsWidget);
    virtual ~ScriptRunner();

    TurtleCanvasGraphicsItem* graphicsWidget() const;

    void requestThreadStop();

    void pauseScript();
    void resumeScript();
    void haltScript();

    void setRequirePaths(const QString& paths);
    void addRequirePath(const QString& path);

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
    void applyRequirePaths();
    void doSleep(int msecs);
    bool haltRequested() const;
    void haltIfRequested();
    void pauseIfRequested();
    void emitMessage(const QString& message);

    void openRestrictedBaseModule();
    void openRestrictedOsModule();

    void setupCommands();

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
    static int sleep(lua_State* state);

    static void debugHookEntry(lua_State* state, lua_Debug* );


    lua_State* m_state;
    TurtleCanvasGraphicsItem* m_graphicsWidget;

    mutable QMutex m_luaMutex; // locked while a script is running

    // Used to send Lua scripts to the thread to be run.
    // See runScript() and run().
    QSemaphore m_scriptsQueueSema;
    mutable QMutex m_scriptsQueueMutex;
    QQueue<QString> m_scriptsQueue;

    // Used to pause the script.
    QWaitCondition m_pauseCond; // The Lua thread waits on this while m_pause==true
    mutable QMutex m_pauseMutex;
    volatile bool m_pause;

    // Flag to tell the script to halt immediately.
    mutable QMutex m_haltMutex;
    volatile bool m_halt;

    // Used to pass scripts' print() messages back to the UI.
    // See emitMessage() and pendingScriptMessage()
    QMutex m_scriptMessageMutex;
    QWaitCondition m_scriptMessageCond;
    QString m_scriptMessage;
    volatile bool m_scriptMessagePending;

    // These are used to implement an interruptable "sleep()" function in lua.
    // The sleep needs to be interruptable so that we can always halt the script,
    // even while it is sleeping (possibly with a huge timeout).
    //
    // The Lua thread uses m_sleepCond.wait() with a timeout to implement the
    // sleep delay. The sleeping thread can be awoken early by calling m_sleepCond.notifyAll().
    QMutex m_sleepMutex;
    QWaitCondition m_sleepCond;
    volatile bool m_sleepAllowed;

    // Require paths management.
    QMutex m_requirePathsMutex;
    QString m_requirePaths;
    volatile bool m_requirePathsChanged;
};

#endif // COMMANDRUNNER_H
