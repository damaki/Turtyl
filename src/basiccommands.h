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
#ifndef BASICCOMMANDS_H
#define BASICCOMMANDS_H

#include "lua.hpp"

class CommandRunner;

void setupCommands(lua_State* state, CommandRunner* parent);

int drawLine(lua_State* state);
int drawArc(lua_State* state);
int clear(lua_State* state);
int setBackgroundColor(lua_State* state);

#endif // BASICCOMMANDS_H
