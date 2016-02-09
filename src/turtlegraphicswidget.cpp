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
#include "turtlegraphicswidget.h"
#include <QMutexLocker>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyleOptionGraphicsItem>
#include <cassert>

static const int DEFAULT_SIZE = 2048;

TurtleGraphicsItem::TurtleGraphicsItem() :
    m_mutex(),
    m_pixmap(DEFAULT_SIZE, DEFAULT_SIZE),
    m_backgroundColor(Qt::white),
    m_antialiased(false)
{
    // We need to call update() each time the canvas is updated (i.e. drawn on)
    // but update() needs to be called by the UI thread, so a queued signal is used.
    connect(this, SIGNAL(canvasUpdated()),
            this, SLOT(callUpdate()),
            Qt::QueuedConnection);

    m_pixmap.fill(Qt::transparent);

    const qreal newpos = static_cast<qreal>(DEFAULT_SIZE) / 2.0;
    setPos(-newpos, -newpos);
}

bool TurtleGraphicsItem::antialiased() const
{
    QMutexLocker lock(&m_mutex);
    return m_antialiased;
}

void TurtleGraphicsItem::setAntialiased(const bool on)
{
    QMutexLocker lock(&m_mutex);
    m_antialiased = on;
}

/**
 * @brief Set the canvas background color.
 *
 * Changing the background color does not affect the current drawings.
 * I.e. the current drawings are rendered unchanged over the new background.
 *
 * @param color The new background color.
 */
void TurtleGraphicsItem::setBackgroundColor(const QColor& color)
{
    {
        QMutexLocker lock(&m_mutex);
        m_backgroundColor = color;
    }

    emit canvasUpdated();
}

/**
 * @brief Clear all drawings on the canvas.
 *
 * The canvasUpdated() signal is emitted after the canvas is cleared.
 */
void TurtleGraphicsItem::clear()
{
    {
        QMutexLocker lock(&m_mutex);
        m_pixmap.fill(Qt::transparent);
    }

    emit canvasUpdated();
}

/**
 * @brief Draw a line on the canvas.
 *
 * The canvasUpdated() signal is emitted after the line is drawn.
 *
 * @param[in] line The line to draw.
 * @param[in] pen The pen to use for drawing the line.
 */
void TurtleGraphicsItem::drawLine(const QLineF &line, const QPen &pen)
{
    {
        QMutexLocker lock(&m_mutex);

        QPainter painter(&m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, m_antialiased);

        painter.setPen(pen);

        // Translate the origin from the user's perspective (center of the drawing area)
        // to QPixmap's origin (top-left of the pixmap).
        painter.drawLine(line.translated(static_cast<qreal>(m_pixmap.width())  / 2.0,
                                           static_cast<qreal>(m_pixmap.height()) / 2.0));
    }

    emit canvasUpdated();
}

/**
 * @brief Draws a circular arc.
 *
 * The canvasUpdated() signal is emitted after the arc is drawn.
 *
 * @param startPos The position of the arc's center.
 * @param startAngle The starting angle of the arc. 0 degrees is aiming straight up.
 * @param angle The angle of the arc (positive values turn clockwise).
 * @param radius The radius of the arc.
 * @param pen The pen to use for drawing the arc.
 */
void TurtleGraphicsItem::drawArc(const QPointF &startPos,
                                   qreal startAngle,
                                   qreal angle,
                                   qreal radius,
                                   const QPen &pen)
{
    {
        QMutexLocker lock(&m_mutex);

        QPainter painter(&m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, m_antialiased);

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

    emit canvasUpdated();
}

/**
 * @brief Get the canvas size.
 *
 * @return The canvas size.
 */
int TurtleGraphicsItem::size() const
{
    QMutexLocker lock(&m_mutex);
    return m_pixmap.width();
}

/**
 * @brief Change the canvas size.
 *
 * The canvasResized() signal is emitted after after the canvas is resized.
 *
 * @param newSize The size (in pixels) of the new canvas. If the size is the same
 *     as the current size then this method has no effect.
 */
void TurtleGraphicsItem::resize(int newSize)
{
    bool wasResized = false;

    {
        QMutexLocker lock(&m_mutex);

        int oldSize = m_pixmap.width();

        if (newSize != oldSize)
        {
            prepareGeometryChange();

            QPixmap newpixmap(newSize, newSize);
            newpixmap.fill(Qt::transparent);

            QPainter painter(&newpixmap);
            int offset;

            if (newSize > oldSize)
            {
                offset = (newSize - oldSize) / 2;
            }
            else
            {
                offset = -((oldSize - newSize) / 2);
            }

            painter.drawPixmap(QPoint(offset, offset),
                               m_pixmap,
                               m_pixmap.rect());

            m_pixmap = newpixmap;

            update();

            const qreal newpos = static_cast<qreal>(newSize) / 2.0;
            setPos(-newpos, -newpos);

            wasResized = true;
        }
    }

    if (wasResized)
    {
        emit canvasResized();
    }
}

QRectF TurtleGraphicsItem::boundingRect() const
{
    return m_pixmap.rect();
}

void TurtleGraphicsItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *,
                               QWidget *)
{
    QMutexLocker lock(&m_mutex);
    painter->fillRect(boundingRect(), m_backgroundColor);
    painter->drawPixmap(boundingRect(), m_pixmap, m_pixmap.rect());
}

/**
 * @brief Calls this->update()
 *
 * The purpose of this method is to allow update() to be indirectly called
 * by the UI thread by a queued signal.
 */
void TurtleGraphicsItem::callUpdate()
{
    update();
}
