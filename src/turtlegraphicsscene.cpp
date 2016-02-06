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
#include "turtlegraphicsscene.h"
#include <QGraphicsItem>
#include <QMutexLocker>

static const int DEFAULT_SIZE = 2048;

TurtleGraphicsScene::TurtleGraphicsScene() :
    m_scene(),
    m_pixmap(DEFAULT_SIZE, DEFAULT_SIZE)

{
    clear();
}


QGraphicsScene& TurtleGraphicsScene::scene()
{
    return m_scene;
}

const QGraphicsScene& TurtleGraphicsScene::scene() const
{
    return m_scene;
}

void TurtleGraphicsScene::clear()
{
    QMutexLocker lock(&m_mutex);

    m_pixmap.fill(Qt::black);
}

void TurtleGraphicsScene::drawLine(const QLineF& line, const QPen& pen)
{
    QMutexLocker lock(&m_mutex);

    QPainter painter(&m_pixmap);

    painter.setPen(pen);

    // Translate the origin from the user's perspective (center of the drawing area)
    // to QPixmap's origin (top-left of the pixmap).
    painter.drawLine(line.translated(static_cast<qreal>(m_pixmap.width())  / 2.0,
                                     static_cast<qreal>(m_pixmap.height()) / 2.0));
}

void TurtleGraphicsScene::drawArc(const QPointF& startPos,
                                  qreal startAngle,
                                  qreal angle,
                                  qreal radius,
                                  const QPen& pen)
{
    QMutexLocker lock(&m_mutex);

    QPainter painter(&m_pixmap);

    painter.setPen(pen);

    QRectF boundingBox(startPos.x() - radius,
                       startPos.y() - radius,
                       radius * 2.0,
                       radius * 2.0);

    // Translate the origin from the user's perspective (center of the drawing area)
    // to QPixmap's origin (top-left of the pixmap).
    boundingBox.translate(static_cast<qreal>(m_pixmap.width())  / 2.0,
                          static_cast<qreal>(m_pixmap.height()) / 2.0);

    // From the user's point of view arcs are drawn clockwise, but Qt draws them
    // counter-clockwise.
    angle      = -angle;

    // There's a difference of 90 degrees from the user's point of view and Qt
    startAngle += 90.0;

    // Angles given to drawArc() are integers representing 1/16th a degree.
    const int startAngleInt = static_cast<int>(startAngle * 16.0);
    const int angleInt      = static_cast<int>(angle * 16.0);

    painter.drawArc(boundingBox, startAngleInt, angleInt);
}

void TurtleGraphicsScene::updateScene()
{
    QMutexLocker lock(&m_mutex);
    m_scene.clear();
    QGraphicsItem* item = m_scene.addPixmap(m_pixmap);

    // Ensure the center of the pixmap is at the center of the scene
    item->setPos(-static_cast<qreal>(m_pixmap.width()) / 2.0,
                 -static_cast<qreal>(m_pixmap.height()) / 2.0);
}
