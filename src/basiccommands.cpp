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
#include "basiccommands.h"
#include "commandrunner.h"
#include <cassert>
#include <QPen>

static const int DRAW_LINE_ARGS_COUNT = 9;
static const int DRAW_ARC_ARGS_COUNT = 11;
static const int SET_BACKGROUND_COLOR_ARGS_COUNT = 3;

static const char* LUA_PARENT_NAME = "_turtyl_parent";


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
static CommandRunner& getParent(lua_State* state)
{
    lua_getglobal(state, LUA_PARENT_NAME);
    void* rawparent = lua_touserdata(state, -1);
    if (rawparent == NULL)
    {
        lua_pushstring(state, "missing parent");
        lua_error(state);
    }

    return *reinterpret_cast<CommandRunner*>(rawparent);
}

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

/**
 * @brief Lua debug hook.
 *
 * This hook is used to pause/resume, and halt the current script.
 */
static void debugHook(lua_State* state, lua_Debug* )
{
    CommandRunner& parent = getParent(state);

    parent.checkPause();

    if (parent.isHaltRequested())
    {
        // Stop the script
        lua_pushstring(state, "halted");
        lua_error(state);
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
void setupCommands(lua_State* state, CommandRunner* parent)
{
    if (NULL != parent)
    {
        lua_pushlightuserdata(state, parent);
        lua_setglobal(state, LUA_PARENT_NAME);
    }

    lua_sethook(state, &debugHook, LUA_MASKCOUNT, 1000);

    lua_register(state, "draw_line", &drawLine);
    lua_register(state, "draw_arc",  &drawArc);
    lua_register(state, "clear", &clear);
    lua_register(state, "set_background_color", &setBackgroundColor);
    lua_register(state, "get_background_color", &getBackgroundColor);
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
int drawLine(lua_State* state)
{
    lua_Number x1,y1;
    lua_Number x2,y2;
    lua_Number r,g,b,a;
    lua_Number size;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_LINE_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_line");
        lua_error(state);
    }

    x1   = getNumber(state, 1, "draw_line()");
    y1   = getNumber(state, 2, "draw_line()");
    x2   = getNumber(state, 3, "draw_line()");
    y2   = getNumber(state, 4, "draw_line()");
    r    = getNumber(state, 5, "draw_line()");
    g    = getNumber(state, 6, "draw_line()");
    b    = getNumber(state, 7, "draw_line()");
    a    = getNumber(state, 8, "draw_line()");
    size = getNumber(state, 9, "draw_line()");

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QLineF line(x1, -y1,
                x2, -y2);

    QPen pen(clippedColor(r,g,b,a), size);

    getParent(state).graphicsWidget()->drawLine(line, pen);

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
 *   10. The thickness of the arc.
 *
 * @note If an error occurs then @c lua_error is called and this function does not return.
 *
 * @param[in,out] state The lua state.
 * @return Returns 0 always. No values are returned to Lua.
 */
int drawArc(lua_State* state)
{
    lua_Number centerx, centery;
    lua_Number startAngle;
    lua_Number angle;
    lua_Number xradius;
    lua_Number yradius;
    lua_Number r,g,b,a;
    lua_Number size;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_ARC_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_arc");
        lua_error(state);
    }

    centerx     = getNumber(state, 1,  "draw_arc()");
    centery     = getNumber(state, 2,  "draw_arc()");
    startAngle  = getNumber(state, 3,  "draw_arc()");
    angle       = getNumber(state, 4,  "draw_arc()");
    xradius     = getNumber(state, 5,  "draw_arc()");
    yradius     = getNumber(state, 6,  "draw_arc()");
    r           = getNumber(state, 7,  "draw_arc()");
    g           = getNumber(state, 8,  "draw_arc()");
    b           = getNumber(state, 9,  "draw_arc()");
    a           = getNumber(state, 10, "draw_arc()");
    size        = getNumber(state, 11, "draw_arc()");

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QPoint arcCenterPos(centerx, -centery);

    QPen pen(clippedColor(r,g,b,a), size);

    getParent(state).graphicsWidget()->drawArc(arcCenterPos, startAngle, angle, xradius, yradius, pen);

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
int clear(lua_State* state)
{
    getParent(state).graphicsWidget()->clear();
    return 0;
}

int setBackgroundColor(lua_State* state)
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

    getParent(state).graphicsWidget()->setBackgroundColor(color);

    return 0;
}

int getBackgroundColor(lua_State* state)
{
    // Clear the stack
    lua_pop(state, lua_gettop(state));

    const QColor color = getParent(state).graphicsWidget()->backgroundColor();

    lua_pushinteger(state, color.red());
    lua_pushinteger(state, color.green());
    lua_pushinteger(state, color.blue());

    return 3;
}
