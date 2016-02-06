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
#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <QGraphicsScene>
#include "turtlegraphicsscene.h"
#include "lua.hpp"

/**
 * @brief Runs commands from Lua scripts.
 *
 * Errors produced when either loading/compiling or running a script
 * produce an error message via the @c commandError() signal.
 */
class CommandRunner : public QObject
{
    Q_OBJECT

public:
    CommandRunner(TurtleGraphicsScene* scene);

    void runCommand(const QString& command);

    void runScriptFile(const QString& filename);

signals:
    void commandError(const QString& message);

private:
    lua_State* m_state;
    TurtleGraphicsScene* m_scene;
};

#endif // COMMANDRUNNER_H
