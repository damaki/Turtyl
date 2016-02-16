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
#include "scriptrunner.h"
#include <QMutexLocker>
#include <QPen>
#include <cassert>

static const int DRAW_LINE_ARGS_COUNT = 10;
static const int DRAW_ARC_ARGS_COUNT = 12;
static const int SET_BACKGROUND_COLOR_ARGS_COUNT = 3;

static const char* LUA_SCRIPT_RUNNER_NAME = "_turtyl_script_runner";


/**
 * @brief Get the @c CommandRunner associated with a Lua VM.
 *
 * If the script does not have a CommandRunner associated with it then
 * @c lua_error is called and this function does not return.
 *
 * @warning This function must only be called from C functions called from Lua.
 * This is because this function calls @c lua_error if an error occurs.
 *
 * @param state The lua state.
 * @return The @c CommandRunner read from the lua state.
 */
static ScriptRunner& getScriptRunner(lua_State* state)
{
    lua_getglobal(state, LUA_SCRIPT_RUNNER_NAME);
    void* rawparent = lua_touserdata(state, -1);
    if (rawparent == NULL)
    {
        lua_pushstring(state, "Could not load ScriptRunner");
        lua_error(state);
    }

    return *reinterpret_cast<ScriptRunner*>(rawparent);
}

/**
 * @brief Get a number from the lua stack, or call lua_error if the stack argument isn't a number.
 *
 * @warning If the value at the specified position on the lua stack is not a number, then
 * lua_error is called and this function does not return.
 *
 * @param state
 * @param stackPos The index of the lua stack from where the number should be read
 * @param funcName The name of the calling function (as it appears from lua). This is used
 *    in the error message.
 * @return The lua number.
 */
static lua_Number getNumber(lua_State* state, int stackPos, const char* funcName)
{
    int isNumber = 0;
    lua_Number number = lua_tonumberx(state, stackPos, &isNumber);
    if (!isNumber)
    {
        lua_pushstring(state,
                       QString("argument %1 to %2 must be a number")
                        .arg(stackPos)
                        .arg(funcName)
                        .toStdString().c_str());
        lua_error(state);
    }
    return number;
}

/**
 * @brief Get an integer from the lua stack, or call lua_error if the stack argument isn't an integer.
 *
 * @warning If the value at the specified position on the lua stack is not an integer, then
 * lua_error is called and this function does not return.
 *
 * @param state
 * @param stackPos The index of the lua stack from where the integer should be read
 * @param funcName The name of the calling function (as it appears from lua). This is used
 *    in the error message.
 * @return The lua integer.
 */
static lua_Integer getInteger(lua_State* state, int stackPos, const char* funcName)
{
    int isInteger = 0;
    lua_Number integer = lua_tointegerx(state, stackPos, &isInteger);
    if (!isInteger)
    {
        lua_pushstring(state,
                       QString("argument %1 to %2 must be an integer")
                        .arg(stackPos)
                        .arg(funcName)
                        .toStdString().c_str());
        lua_error(state);
    }
    return integer;
}

/**
 * @brief Return a QColor from real RGBA components.
 *
 * If any of the RGBA components are outside the range [0,255] then
 * they are clipped to the valid range. E.g. the value 300 is clipped
 * to 255 and -10 is clipped to 0.
 *
 * The qreal values are rounded to the nearest integer value during
 * the conversion.
 *
 * @param r
 * @param g
 * @param b
 * @param a
 * @return
 */
static QColor clippedColor(qreal r, qreal g, qreal b, qreal a)
{
    // Add 0.5 to round to the nearest integer color value.
    return QColor(static_cast<int>(std::min(255.0, std::max(0.0, r + 0.5))),
                  static_cast<int>(std::min(255.0, std::max(0.0, g + 0.5))),
                  static_cast<int>(std::min(255.0, std::max(0.0, b + 0.5))),
                  static_cast<int>(std::min(255.0, std::max(0.0, a + 0.5))));
}

static void setPenCapStyle(QPen& pen, lua_Integer capStyle)
{
    switch (capStyle)
    {
    case 2:
        pen.setCapStyle(Qt::FlatCap);
        break;

    case 3:
        pen.setCapStyle(Qt::RoundCap);
        break;

    case 1:
    default:
        pen.setCapStyle(Qt::SquareCap);

    }
}


ScriptRunner::ScriptRunner(TurtleCanvasGraphicsItem* const graphicsWidget) :
    m_state(luaL_newstate()),
    m_graphicsWidget(graphicsWidget),
    m_scriptsQueueSema(),
    m_scriptsQueueMutex(),
    m_scriptsQueue(),
    m_pauseCond(),
    m_pauseMutex(),
    m_pause(false),
    m_haltMutex(),
    m_halt(false),
    m_scriptMessageMutex(),
    m_scriptMessageCond(),
    m_scriptMessage(),
    m_scriptMessagePending(false),
    m_requirePathsMutex(),
    m_requirePaths(),
    m_requirePathsChanged(false)
{
    assert(NULL != m_state);
    assert(NULL != graphicsWidget);

    // Load all libraries except io and debug.
    // These libraries are omitted for security.
    luaL_requiref(m_state, "coroutine", &luaopen_coroutine, 1);
    luaL_requiref(m_state, "math",      &luaopen_math,      1);
    luaL_requiref(m_state, "package",   &luaopen_package,   1);
    luaL_requiref(m_state, "string",    &luaopen_string,    1);
    luaL_requiref(m_state, "table",     &luaopen_table,     1);
    luaL_requiref(m_state, "utf8",      &luaopen_utf8,      1);
    lua_pop(m_state, 6);

    openRestrictedBaseModule();
    openRestrictedOsModule();

    setupCommands();

    applyRequirePaths();
}

TurtleCanvasGraphicsItem* ScriptRunner::graphicsWidget() const
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
    m_scriptsQueueSema.release(); // wake up the thread (if it's sleeping)
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

    // Don't allow the Lua threads to sleep().
    {
        QMutexLocker lock(&m_sleepMutex);
        m_sleepAllowed = false;
        m_sleepCond.wakeAll();
    }

    // The script might be waiting to send a message.
    // If so, then wake it up, so that it can detect the halt request.
    // (see emitMessage())
    m_scriptMessageCond.wakeAll();

    // The command might currently be paused.
    resumeScript();
}

/**
 * @brief Sets lua's package.path to the specified string.
 *
 * @note See the lua documentation for package.path and package.searchers
 * for acceptable string formats.
 *
 * @note The new require paths are applied the next time a script is
 * run. It does not affect the currently running script.
 *
 * @param path Paths to search. E.g. "/?/init.lua;./scripts/?.lua"
 */
void ScriptRunner::setRequirePaths(const QString& paths)
{
    QMutexLocker lock(&m_requirePathsMutex);
    m_requirePaths = paths;
    m_requirePathsChanged = true;
}

/**
 * @brief Appends a path to be searched by lua when a module is loaded.
 *
 * @note See the lua documentation for package.path and package.searchers
 * for acceptable string formats.
 *
 * @note The new require paths are applied the next time a script is
 * run. It does not affect the currently running script.
 *
 * @param path The path to append.
 */
void ScriptRunner::addRequirePath(const QString& path)
{
    QMutexLocker lock(&m_requirePathsMutex);
    if (m_requirePaths.isEmpty())
    {
        m_requirePaths = path;
    }
    else
    {
        m_requirePaths += ';' + path;
    }
    m_requirePathsChanged = true;
}

/**
 * @brief Executes a Lua script script.
 *
 * If an error occurs then the @c commandError signal is emitted.
 *
 * @param[in] command String containing the Lua code to execute.
 */
void ScriptRunner::runScript(const QString& script)
{
    {
        QMutexLocker lock(&m_sleepMutex);
        m_sleepAllowed = true;
    }

    {
        QMutexLocker lock(&m_pauseMutex);
        m_pause = false;
    }

    {
        QMutexLocker lock(&m_haltMutex);
        m_halt = false;
    }

    {
        QMutexLocker lock(&m_scriptsQueueMutex);
        m_scriptsQueue.push_back(script);
    }

    m_scriptsQueueSema.release();
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
    {
        QMutexLocker lock(&m_sleepMutex);
        m_sleepAllowed = true;
    }

    QMutexLocker lock(&m_luaMutex);

    applyRequirePaths();

    lua_pop(m_state, lua_gettop(m_state));
    if (0 == luaL_dofile(m_state, filename.toStdString().c_str()))
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
 * @brief Update Lua's @c package.path with the updated require paths.
 *
 * If the require paths have not been changed since the last time
 * applyRequirePaths() was called then this method has no effect.
 *
 * @see setRequirePaths()
 * @see addRequirePath()
 *
 * @pre @c m_luaMutex is locked by the caller.
 */
void ScriptRunner::applyRequirePaths()
{
    QMutexLocker lock(&m_requirePathsMutex);
    if (m_requirePathsChanged)
    {
        m_requirePathsChanged = false;

        lua_pop(m_state, lua_gettop(m_state));

        if (!lua_getglobal(m_state, "package"))
        {
            return;
        }

        lua_pushstring(m_state, m_requirePaths.toStdString().c_str());
        lua_setfield(m_state, 1, "path");

        lua_pop(m_state, lua_gettop(m_state));
    }
}

void ScriptRunner::doSleep(int msecs)
{
    if (msecs > 0)
    {
        QMutexLocker lock(&m_sleepMutex);
        if (m_sleepAllowed)
        {
            (void)m_sleepCond.wait(&m_sleepMutex, msecs);
        }
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

/**
 * @brief Halt the current script if it was requested.
 *
 * @warning This function will not return if it halts the script, since it calls
 * lua_error().
 */
void ScriptRunner::haltIfRequested()
{
    if (haltRequested())
    {
        lua_error(m_state);
    }
}

/**
 * @brief This method blocks while the script is paused.
 */
void ScriptRunner::pauseIfRequested()
{
    QMutexLocker lock(&m_pauseMutex);
    while (m_pause)
    {
        m_pauseCond.wait(&m_pauseMutex);
    }
}

void ScriptRunner::emitMessage(const QString& message)
{
    QMutexLocker lock(&m_scriptMessageMutex);

    // If the UI hasn't yet read the previous message then wait for the UI to
    // catch up before sending the next one to avoid overloading the UI.
    while ((!haltRequested()) && m_scriptMessagePending)
    {
        m_scriptMessageCond.wait(&m_scriptMessageMutex);
    }

    // Don't do anything if the script needs to halt.
    if (!haltRequested())
    {
        m_scriptMessage = message;
        m_scriptMessagePending = true;

        emit scriptMessageReceived();
    }
}

/**
 * @brief Get all pending messages printed by the script.
 *
 * @return A queue containing each individual message printed by the script.
 */
QString ScriptRunner::pendingScriptMessage()
{
    QString message;

    QMutexLocker lock(&m_scriptMessageMutex);
    message.swap(m_scriptMessage);
    m_scriptMessagePending = false;
    m_scriptMessageCond.wakeAll();

    return message;
}

void ScriptRunner::clearPendingScriptMessage()
{
    QMutexLocker lock(&m_scriptMessageMutex);
    m_scriptMessage.clear();
    m_scriptMessagePending = false;
    m_scriptMessageCond.wakeAll();
}

void ScriptRunner::run()
{
    QString scriptData;
    bool hasScriptData;

    while (!isInterruptionRequested())
    {
        m_scriptsQueueSema.acquire();

        if (isInterruptionRequested())
        {
            break;
        }

        // Check if there's any scripts in the queue
        {
            QMutexLocker lock(&m_scriptsQueueMutex);
            if (m_scriptsQueue.size() > 0)
            {
                scriptData = m_scriptsQueue.front();
                m_scriptsQueue.pop_front();
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
            QMutexLocker lock(&m_luaMutex);

            applyRequirePaths();

            lua_pop(m_state, lua_gettop(m_state));

            if (0 == luaL_dostring(m_state, scriptData.toStdString().c_str()))
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

/**
 * @brief Load's lua's base module, but without unsafe functions.
 *
 * The following functions are @b removed from the base module:
 *    - @c dofile
 *    - @c load
 *    - @c loadfile
 */
void ScriptRunner::openRestrictedBaseModule()
{
    // Ensure a clean stack
    lua_pop(m_state, lua_gettop(m_state));

    luaL_requiref(m_state, "_G", &luaopen_base, 1);

    // _G.dofile = nil
    lua_pushnil(m_state);
    lua_setglobal(m_state, "dofile");

    // _G.load = nil
    lua_pushnil(m_state);
    lua_setglobal(m_state, "load");

    // _G.loadfile = nil
    lua_pushnil(m_state);
    lua_setglobal(m_state, "loadfile");

    lua_pop(m_state, lua_gettop(m_state));
}

/**
 * @brief Load's lua's os module, but without unsafe functions.
 *
 * The following functions are @b removed from the os module:
 *    - @c os.execute
 *    - @c os.exit
 *    - @c os.getenv
 *    - @c os.remove
 *    - @c os.rename
 *    - @c os.setlocale
 *    - @c os.tmpname
 */
void ScriptRunner::openRestrictedOsModule()
{
    // Ensure a clean stack
    lua_pop(m_state, lua_gettop(m_state));

    // Load the os module, but remove functions that could modify the host system.
    // This should provide protection against malicious scripts.
    luaopen_os(m_state);

    // os.execute = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "execute");

    // os.exit = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "exit");

    // os.getenv = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "getenv");

    // os.remove = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "remove");

    // os.rename = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "rename");

    // os.setlocale = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "setlocale");

    // os.tmpname = nil
    lua_pushnil(m_state);
    lua_setfield(m_state, 1, "tmpname");

    lua_setglobal(m_state, "os");

    lua_pop(m_state, lua_gettop(m_state));
}

/**
 * @brief Setup the commands for a lua state.
 *
 * The pointer to the @p scene is stored in the lua state. All drawing
 * commands act on this scene. The scene must remain valid for the lifetime
 * of the lua state.
 *
 * @param[in,out] state The commands are registered to this lua state.
 * @param[in] scene The scene to associate with the lua state.
 */
void ScriptRunner::setupCommands()
{
    static const luaL_Reg uiTableFuncs[] =
    {
        "print", &ScriptRunner::printMessage
    };

    static const luaL_Reg canvasTableFuncs[] =
    {
        "drawline",           &ScriptRunner::drawLine,
        "drawarc",            &ScriptRunner::drawArc,
        "clear",              &ScriptRunner::clearScreen,
        "setbackgroundcolor", &ScriptRunner::setBackgroundColor,
        "getbackgroundcolor", &ScriptRunner::getBackgroundColor,
        "setturtle",          &ScriptRunner::setTurtle,
        "getturtle",          &ScriptRunner::getTurtle,
        "showturtle",         &ScriptRunner::showTurtle,
        "hideturtle",         &ScriptRunner::hideTurtle,
        "turtlehidden",       &ScriptRunner::turtleHidden
    };

    lua_settop(m_state, 0); // ensure empty stack

    // Add ourself to the script so that we can retrieve our
    // 'this' pointer when lua calls one of our functions.
    lua_pushlightuserdata(m_state, this);
    lua_setglobal(m_state, LUA_SCRIPT_RUNNER_NAME);

    luaL_newlib(m_state, uiTableFuncs);

    lua_pushliteral(m_state, "canvas");
    luaL_newlib(m_state, canvasTableFuncs);

    lua_rawset(m_state, 1); // _ui['canvas'] = canvas

    lua_setglobal(m_state, "_ui"); // _G['_ui'] = _ui

    lua_register(m_state, "sleep", &ScriptRunner::sleep);

    lua_sethook(m_state, &debugHookEntry, LUA_MASKCOUNT, 100);
}

void ScriptRunner::debugHook(lua_State* state)
{
    assert(state == m_state);

    pauseIfRequested();

    haltIfRequested();
}

/**
 * @brief Draws a line.
 *
 * This function receives 8 parameters from lua, all of which are lua numbers:
 *   1. The x coordinate of the line's starting point.
 *   2. The y coordinate of the line's starting point.
 *   3. The x coordinate of the line's ending point.
 *   4. The y coordinate of the line's ending point.
 *   5. The R component of the line's RGB color.
 *   6. The G component of the line's RGB color.
 *   7. The B component of the line's RGB color.
 *   8. The thickness of the line.
 *
 * @note If an error occurs then @c lua_error is called and this function does not return.
 *
 * @param[in,out] state The lua state.
 * @return Returns 0 always. No values are returned to Lua.
 */
int ScriptRunner::drawLine(lua_State* state)
{
    lua_Number x1,y1;
    lua_Number x2,y2;
    lua_Number r,g,b,a;
    lua_Number size;
    lua_Integer capStyle;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_LINE_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_line");
        lua_error(state);
    }

    x1       = getNumber (state, 1,  "_ui.drawline()");
    y1       = getNumber (state, 2,  "_ui.drawline()");
    x2       = getNumber (state, 3,  "_ui.drawline()");
    y2       = getNumber (state, 4,  "_ui.drawline()");
    r        = getNumber (state, 5,  "_ui.drawline()");
    g        = getNumber (state, 6,  "_ui.drawline()");
    b        = getNumber (state, 7,  "_ui.drawline()");
    a        = getNumber (state, 8,  "_ui.drawline()");
    size     = getNumber (state, 9,  "_ui.drawline()");
    capStyle = getInteger(state, 10, "_ui.drawline()");

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QLineF line(x1, -y1,
                x2, -y2);

    QPen pen(clippedColor(r,g,b,a), size);
    setPenCapStyle(pen, capStyle);

    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->drawLine(line, pen);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

/**
 * @brief Draws an arc.
 *
 * This function receives 6 parameters from lua, all of which are lua numbers:
 *   1. The x coordinate of the arc's center.
 *   2. The y coordinate of the arc's center.
 *   3. The starting angle of the arc.
 *   4. The arc's span angle.
 *   5. The arc's X radius
 *   6. The arc's Y radius
 *   7. The R component of the arc's RGBA color.
 *   8. The G component of the arc's RGBA color.
 *   9. The B component of the arc's RGBA color.
 *   10. The A component of the arc's RGBA color.
 *   11. The thickness of the arc.
 *   12. The pen's cap style.
 *
 * @note If an error occurs then @c lua_error is called and this function does not return.
 *
 * @param[in,out] state The lua state.
 * @return Returns 0 always. No values are returned to Lua.
 */
int ScriptRunner::drawArc(lua_State* state)
{
    lua_Number centerx, centery;
    lua_Number startAngle;
    lua_Number angle;
    lua_Number xradius;
    lua_Number yradius;
    lua_Number r,g,b,a;
    lua_Number size;
    lua_Integer capStyle;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_ARC_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_arc");
        lua_error(state);
    }

    centerx     = getNumber (state, 1,  "_ui.drawarc()");
    centery     = getNumber (state, 2,  "_ui.drawarc()");
    startAngle  = getNumber (state, 3,  "_ui.drawarc()");
    angle       = getNumber (state, 4,  "_ui.drawarc()");
    xradius     = getNumber (state, 5,  "_ui.drawarc()");
    yradius     = getNumber (state, 6,  "_ui.drawarc()");
    r           = getNumber (state, 7,  "_ui.drawarc()");
    g           = getNumber (state, 8,  "_ui.drawarc()");
    b           = getNumber (state, 9,  "_ui.drawarc()");
    a           = getNumber (state, 10, "_ui.drawarc()");
    size        = getNumber (state, 11, "_ui.drawarc()");
    capStyle    = getInteger(state, 12, "_ui.drawarc()");

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QPoint arcCenterPos(centerx, -centery);

    QPen pen(clippedColor(r,g,b,a), size);
    setPenCapStyle(pen, capStyle);

    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->drawArc(arcCenterPos, startAngle, angle, xradius, yradius, pen);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

/**
 * @brief Clear the screen.
 *
 * @note If an error occurs then @c lua_error is called and this function does not return.
 *
 * @param state The lua state.
 * @return Returns 0 always. No values are returned to Lua.
 */
int ScriptRunner::clearScreen(lua_State* state)
{
    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->clear();
    runner.pauseIfRequested();
    runner.haltIfRequested();
    return 0;
}

int ScriptRunner::setBackgroundColor(lua_State* state)
{
    lua_Number r,g,b;

    // Check number of arguments
    if (lua_gettop(state) < SET_BACKGROUND_COLOR_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to bgcolor()");
        lua_error(state);
    }

    // Read each argument from the Lua stack
    r = getNumber(state, 1, "set_background_color()");
    g = getNumber(state, 2, "set_background_color()");
    b = getNumber(state, 3, "set_background_color()");

    // Note: the background color is always opaque
    QColor color(clippedColor(r,g,b,255.0));

    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->setBackgroundColor(color);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

int ScriptRunner::getBackgroundColor(lua_State* state)
{
    // Clear the stack
    lua_pop(state, lua_gettop(state));

    ScriptRunner& runner = getScriptRunner(state);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    const QColor color = runner.graphicsWidget()->backgroundColor();

    lua_pushinteger(state, color.red());
    lua_pushinteger(state, color.green());
    lua_pushinteger(state, color.blue());

    return 3;
}

int ScriptRunner::setTurtle(lua_State* state)
{
    lua_Number x,y;
    lua_Number heading;
    lua_Number r,g,b,a;

    // Check number of arguments
    if (lua_gettop(state) < SET_BACKGROUND_COLOR_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to bgcolor()");
        lua_error(state);
    }

    // Read each argument from the Lua stack
    x       = getNumber(state, 1, "_ui.setturtle()");
    y       = getNumber(state, 2, "_ui.setturtle()");
    heading = getNumber(state, 3, "_ui.setturtle()");
    r       = getNumber(state, 4, "_ui.setturtle()");
    g       = getNumber(state, 5, "_ui.setturtle()");
    b       = getNumber(state, 6, "_ui.setturtle()");
    a       = getNumber(state, 7, "_ui.setturtle()");

    QPointF pos(x,y);
    QColor color(clippedColor(r,g,b,a));

    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->setTurtle(pos, heading, color);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

int ScriptRunner::getTurtle(lua_State* state)
{
    // Clear the stack
    lua_pop(state, lua_gettop(state));

    QPointF pos;
    qreal heading;
    QColor color;

    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->getTurtle(pos, heading, color);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    lua_pushnumber(state,  pos.x());
    lua_pushnumber(state,  pos.y());
    lua_pushnumber(state,  heading);
    lua_pushinteger(state, color.red());
    lua_pushinteger(state, color.green());
    lua_pushinteger(state, color.blue());
    lua_pushinteger(state, color.alpha());

    return 7;
}

int ScriptRunner::showTurtle(lua_State* state)
{
    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->showTurtle();
    runner.pauseIfRequested();
    runner.haltIfRequested();
    return 0;
}

int ScriptRunner::hideTurtle(lua_State* state)
{
    ScriptRunner& runner = getScriptRunner(state);
    runner.graphicsWidget()->hideTurtle();
    runner.pauseIfRequested();
    runner.haltIfRequested();
    return 0;
}

int ScriptRunner::turtleHidden(lua_State* state)
{
    ScriptRunner& runner = getScriptRunner(state);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    const bool hidden = runner.graphicsWidget()->turtleHidden();

    lua_pushboolean(state, hidden ? 1 : 0);

    return 1;
}

int ScriptRunner::printMessage(lua_State* state)
{
    QString message;

    const int argc = lua_gettop(state);

    for (int i = 1; i <= argc; i++)
    {
        if (lua_isstring(state, i) || lua_isnumber(state, i))
        {
            message.append(lua_tostring(state, i));
        }
    }

    ScriptRunner& runner = getScriptRunner(state);
    runner.emitMessage(message);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

int ScriptRunner::sleep(lua_State* state)
{
    qreal delay = getNumber(state, 1, "sleep()");

    if (delay < 0.0)
    {
        delay = 0.0;
    }

    delay *= 1000;
    unsigned long msecs = static_cast<unsigned long>(delay);

    ScriptRunner& runner = getScriptRunner(state);
    runner.doSleep(msecs);
    runner.pauseIfRequested();
    runner.haltIfRequested();

    return 0;
}

/**
 * @brief Lua debug hook.
 *
 * This hook is used to pause/resume, and halt the current script.
 */
void ScriptRunner::debugHookEntry(lua_State* state, lua_Debug* )
{
    getScriptRunner(state).debugHook(state);
}
