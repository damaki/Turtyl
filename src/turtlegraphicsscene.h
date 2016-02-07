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
#ifndef TURTLEGRAPHICSVIEW_H
#define TURTLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMutex>

/**
 * @brief Manages drawings using a QGraphicsScene.
 *
 * Drawings are rendered internally into a QPixmap before being displayed in the
 * QGraphicsScene. This keeps memory usage to a minimum in drawings with many
 * (millions) of lines. @c updateScene() must be called after drawing methods have
 * been called to apply the changes to the scene.
 */
class TurtleGraphicsScene
{
public:
    TurtleGraphicsScene();

    QGraphicsScene& scene();
    const QGraphicsScene& scene() const;

    int canvasSize() const;
    void setCanvasSize(int newSize);

    bool antialiasingEnabled() const;
    void setAntialiasing(bool on = true);

    void clear();

    void drawLine(const QLineF& line, const QPen& pen);
    void drawArc(const QPointF& startPos,
                 qreal startAngle,
                 qreal angle,
                 qreal radius,
                 const QPen& pen);

    void updateScene();

private:
    QMutex m_mutex;
    QGraphicsScene m_scene;
    QPixmap m_pixmap;

    QPen m_pen;

    bool m_isAntialiasingEnabled;
};

#endif // TURTLEGRAPHICSVIEW_H
