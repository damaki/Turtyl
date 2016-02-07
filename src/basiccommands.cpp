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
#include "basiccommands.h"
#include <cassert>

static const int DRAW_LINE_ARGS_COUNT = 8;
static const int DRAW_ARC_ARGS_COUNT = 9;
static const int SET_BACKGROUND_COLOR_ARGS_COUNT = 3;

static const char* LUA_SCENE_NAME = "_tartargua_scene";

/**
 * @brief Get the @c TurtleGraphicsScene associated with a Lua VM.
 *
 * If the script does not have a TurtleGraphicsScene associated with it then
 * @c lua_error is called and this function does not return.
 *
 * @warning This function must only be called from C functions called from Lua.
 * This is because this function calls @c lua_error if an error occurs.
 *
 * @param state The lua state.
 * @return The @c TurtleGraphicsScene read from the lua state.
 */
static TurtleGraphicsScene& getGraphicsScene(lua_State* state)
{
    lua_getglobal(state, LUA_SCENE_NAME);
    void* rawscene = lua_touserdata(state, -1);
    if (rawscene == NULL)
    {
        lua_pushstring(state, "missing scene");
        lua_error(state);
    }

    return *reinterpret_cast<TurtleGraphicsScene*>(rawscene);
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
void setupCommands(lua_State* state, TurtleGraphicsScene* scene)
{
    if (NULL != scene)
    {
        lua_pushlightuserdata(state, scene);
        lua_setglobal(state, LUA_SCENE_NAME);
    }

    lua_register(state, "draw_line", &drawLine);
    lua_register(state, "draw_arc",  &drawArc);
    lua_register(state, "clear", &clear);
    lua_register(state, "set_background_color", &setBackgroundColor);
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
    // Args 0..3 are the x1,y1,x2,y2 coordinates of the line
    // Args 4..6 are the RGB color components
    // Arg 7 is the line thickness.
    lua_Number arguments[DRAW_LINE_ARGS_COUNT];
    int isNumber = 0;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_LINE_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_line");
        lua_error(state);
    }

    // Read each argument from the Lua stack
    for (int i = 1; i <= DRAW_LINE_ARGS_COUNT; i++)
    {
        arguments[i-1] = lua_tonumberx(state, i, &isNumber);
        if (!isNumber)
        {
            lua_pushstring(state,
                           QString("argument %1 to draw_line must be a number")
                            .arg(i).toStdString().c_str());
            lua_error(state);
        }
    }

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QLineF line(arguments[0], -arguments[1],
                arguments[2], -arguments[3]);

    arguments[4] = std::min(255.0, std::max(0.0, arguments[4]));
    arguments[5] = std::min(255.0, std::max(0.0, arguments[5]));
    arguments[6] = std::min(255.0, std::max(0.0, arguments[6]));
    QColor color(static_cast<int>(arguments[4]),
                 static_cast<int>(arguments[5]),
                 static_cast<int>(arguments[6]));

    QPen pen(QBrush(color), arguments[7]);

    getGraphicsScene(state).drawLine(line, pen);

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
 *   5. The arc's radius
 *   6. The R component of the arc's RGB color.
 *   7. The G component of the arc's RGB color.
 *   8. The B component of the arc's RGB color.
 *   9. The thickness of the arc.
 *
 * @note If an error occurs then @c lua_error is called and this function does not return.
 *
 * @param[in,out] state The lua state.
 * @return Returns 0 always. No values are returned to Lua.
 */
int drawArc(lua_State* state)
{
    lua_Number arguments[DRAW_ARC_ARGS_COUNT];
    int isNumber = 0;

    // Check number of arguments
    if (lua_gettop(state) < DRAW_ARC_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to draw_arc");
        lua_error(state);
    }

    // Read each argument from the Lua stack
    for (int i = 1; i <= DRAW_ARC_ARGS_COUNT; i++)
    {
        arguments[i-1] = lua_tonumberx(state, i, &isNumber);
        if (!isNumber)
        {
            lua_pushstring(state,
                           QString("argument %1 to draw_arc must be a number")
                            .arg(i).toStdString().c_str());
            lua_error(state);
        }
    }

    // The bottom-left of the screen as it appears to the user is (0,0)
    // In Qt the top-left is (0,0) so the coordinates from the script are flipped
    QPoint arcCenterPos(arguments[0], -arguments[1]);

    qreal startAngle = arguments[2];
    qreal angle      = arguments[3];
    qreal radius     = arguments[4];

    arguments[5] = std::min(255.0, std::max(0.0, arguments[5]));
    arguments[6] = std::min(255.0, std::max(0.0, arguments[6]));
    arguments[7] = std::min(255.0, std::max(0.0, arguments[7]));
    QColor color(static_cast<int>(arguments[5]),
                 static_cast<int>(arguments[6]),
                 static_cast<int>(arguments[7]));

    QPen pen(QBrush(color), arguments[8]);

    getGraphicsScene(state).drawArc(arcCenterPos, startAngle, angle, radius, pen);

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
    getGraphicsScene(state).clear();
    return 0;
}

int setBackgroundColor(lua_State* state)
{
    lua_Number arguments[SET_BACKGROUND_COLOR_ARGS_COUNT];
    int isNumber = 0;

    // Check number of arguments
    if (lua_gettop(state) < SET_BACKGROUND_COLOR_ARGS_COUNT)
    {
        lua_pushstring(state, "too few arguments to bgcolor()");
        lua_error(state);
    }

    // Read each argument from the Lua stack
    for (int i = 1; i <= SET_BACKGROUND_COLOR_ARGS_COUNT; i++)
    {
        arguments[i-1] = lua_tonumberx(state, i, &isNumber);
        if (!isNumber)
        {
            lua_pushstring(state,
                           QString("argument %1 to bgcolor() must be a number")
                            .arg(i).toStdString().c_str());
            lua_error(state);
        }
    }

    arguments[0] = std::min(255.0, std::max(0.0, arguments[0]));
    arguments[1] = std::min(255.0, std::max(0.0, arguments[1]));
    arguments[2] = std::min(255.0, std::max(0.0, arguments[2]));
    QColor color(static_cast<int>(arguments[0]),
                 static_cast<int>(arguments[1]),
                 static_cast<int>(arguments[2]));

    QGraphicsView* view = getGraphicsScene(state).view();
    assert(view != NULL);
    if (view != NULL)
    {
        view->setBackgroundBrush(QBrush(color));
    }

    return 0;
}
