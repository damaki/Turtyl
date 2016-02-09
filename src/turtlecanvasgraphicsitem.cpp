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
#include "turtlecanvasgraphicsitem.h"
#include <QMutexLocker>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyleOptionGraphicsItem>
#include <cassert>

static const int DEFAULT_SIZE = 2048;

TurtleCanvasGraphicsItem::TurtleCanvasGraphicsItem() :
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

bool TurtleCanvasGraphicsItem::antialiased() const
{
    QMutexLocker lock(&m_mutex);
    return m_antialiased;
}

void TurtleCanvasGraphicsItem::setAntialiased(const bool on)
{
    QMutexLocker lock(&m_mutex);
    m_antialiased = on;
}

QColor TurtleCanvasGraphicsItem::backgroundColor() const
{
    QMutexLocker lock(&m_mutex);
    return m_backgroundColor;
}

/**
 * @brief Set the canvas background color.
 *
 * Changing the background color does not affect the current drawings.
 * I.e. the current drawings are rendered unchanged over the new background.
 *
 * @param color The new background color.
 */
void TurtleCanvasGraphicsItem::setBackgroundColor(const QColor& color)
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
void TurtleCanvasGraphicsItem::clear()
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
void TurtleCanvasGraphicsItem::drawLine(const QLineF &line, const QPen &pen)
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
 * @brief Draws an elliptical arc around a point.
 *
 * The canvasUpdated() signal is emitted after the arc is drawn.
 *
 * The coordinate system for drawArc()'s angles are as follows:
 *
 * @code
 *             (0 deg)
 *                 |
 *                 |
 *                 |
 *(+270 deg) ------+------ (+90 deg)
 *                 |
 *                 |
 *                 |
 *            (+180 deg)
 * @endcode
 *
 * An arc of 90 degrees will span clockwise as follows:
 *
 * @code
 *      +-_
 *      |  '-.
 *      |     \
 *      |      |
 *------+------+
 *      |
 *      |
 *      |
 * @endcode
 *
 * Negative angles span counter-clockwise. For example, an arc of -90 degrees:
 * @code
 *      _-+
 *   .-'  |
 *  /     |
 * |      |
 * +------+------
 *        |
 *        |
 *        |
 * @endcode
 *
 * The arc is drawn with (possibly different) X and Y radiuses, which permits
 * drawing ellipses.
 *
 * The @p startAngle parameter controls the rotation of the entire arc.
 *
 * @param centerPos The position of the arc's center on the canvas.
 * @param startAngle The starting angle (degrees) of the arc. 0 degrees is aiming straight up.
 * @param angle The angle (degrees) of the arc (positive values turn clockwise).
 * @param radius The radius of the arc.
 * @param pen The pen to use for drawing the arc.
 */
void TurtleCanvasGraphicsItem::drawArc(const QPointF &centerPos,
                                   qreal startAngle,
                                   qreal angle,
                                   qreal xradius,
                                   qreal yradius,
                                   const QPen &pen)
{
    {
        QMutexLocker lock(&m_mutex);

        QPainter painter(&m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, m_antialiased);

        painter.setPen(pen);

        // Bounding box centered around the origin.
        // This permits rotating the drawing around the origing, based on startAngle.
        QRectF boundingBox(-xradius,
                           -yradius,
                           xradius * 2.0,
                           yradius * 2.0);

        // From the user's point of view arcs are drawn clockwise, but Qt draws them
        // counter-clockwise.
        angle = -angle;

        // Angles given to drawArc() are integers representing 1/16th a degree.
        const int angleInt      = static_cast<int>(angle * 16.0);

        // Translate the origin from the user's perspective (center of the drawing area)
        // to QPixmap's origin (top-left of the pixmap).
        painter.translate(static_cast<qreal>(m_pixmap.width())  / 2.0,
                          static_cast<qreal>(m_pixmap.height()) / 2.0);

        // Translate from the origin to the final position.
        painter.translate(centerPos.x(), centerPos.y());

        // Rotate the entire arc about its center point,
        // also adjust for the 90 degree difference in the coordinate systems.
        painter.rotate(startAngle - 90.0);

        painter.drawArc(boundingBox, 0, angleInt);
    }

    emit canvasUpdated();
}

/**
 * @brief Get the canvas size.
 *
 * @return The canvas size.
 */
int TurtleCanvasGraphicsItem::size() const
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
void TurtleCanvasGraphicsItem::resize(int newSize)
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

QRectF TurtleCanvasGraphicsItem::boundingRect() const
{
    return m_pixmap.rect();
}

void TurtleCanvasGraphicsItem::paint(QPainter *painter,
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
void TurtleCanvasGraphicsItem::callUpdate()
{
    update();
}
