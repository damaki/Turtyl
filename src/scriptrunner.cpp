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


    setupCommands(m_state);
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

    // The command might currently be paused.
    resumeScript();
}

void ScriptRunner::addRequirePath(QString path)
{
    path = path.trimmed().prepend(';');

    QMutexLocker lock(&m_luaMutex);

    // Ensure a clean stack
    lua_pop(m_state, lua_gettop(m_state));

    if (!lua_getglobal(m_state, "package"))
    {
        return;
    }

    if (!lua_getfield(m_state, 1, "path"))
    {
        lua_pop(m_state, 1);
        return;
    }

    lua_pushstring(m_state, path.toStdString().c_str());
    lua_concat(m_state, 2);
    lua_setfield(m_state, 1, "path");

    lua_pop(m_state, lua_gettop(m_state));
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
void ScriptRunner::setupCommands(lua_State* state)
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
        "getbackgroundcolor", &ScriptRunner::getBackgroundColor
    };

    lua_settop(state, 0); // ensure empty stack

    // Add ourself to the script so that we can retrieve our
    // 'this' pointer when lua calls one of our functions.
    lua_pushlightuserdata(state, this);
    lua_setglobal(state, LUA_SCRIPT_RUNNER_NAME);

    luaL_newlib(state, uiTableFuncs);

    lua_pushliteral(state, "canvas");
    luaL_newlib(state, canvasTableFuncs);

    lua_rawset(state, 1); // _ui['canvas'] = canvas

    lua_setglobal(state, "_ui"); // _G['_ui'] = _ui

    lua_sethook(state, &debugHookEntry, LUA_MASKCOUNT, 1000);
}

void ScriptRunner::debugHook(lua_State* state)
{
    // Block if the script is paused.
    {
        QMutexLocker lock(&m_pauseMutex);
        while (m_pause)
        {
            m_pauseCond.wait(&m_pauseMutex);
        }
    }


    if (haltRequested())
    {
        // Stop the script
        lua_pushstring(state, "halted");
        lua_error(state);
    }
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

    getScriptRunner(state).graphicsWidget()->drawLine(line, pen);

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

    getScriptRunner(state).graphicsWidget()->drawArc(arcCenterPos, startAngle, angle, xradius, yradius, pen);

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
    getScriptRunner(state).graphicsWidget()->clear();
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

    getScriptRunner(state).graphicsWidget()->setBackgroundColor(color);

    return 0;
}

int ScriptRunner::getBackgroundColor(lua_State* state)
{
    // Clear the stack
    lua_pop(state, lua_gettop(state));

    const QColor color = getScriptRunner(state).graphicsWidget()->backgroundColor();

    lua_pushinteger(state, color.red());
    lua_pushinteger(state, color.green());
    lua_pushinteger(state, color.blue());

    return 3;
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

    getScriptRunner(state).printMessage(message);

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
