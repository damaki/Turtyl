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
#include <cmath>

static const int DEFAULT_SIZE = 2048;

TurtleCanvasGraphicsItem::TurtleCanvasGraphicsItem() :
    m_mutex(),
    m_pixmap(DEFAULT_SIZE, DEFAULT_SIZE),
    m_backgroundColor(Qt::white),
    m_usedRect(DEFAULT_SIZE/2,DEFAULT_SIZE/2,1,1),
    m_turtlePos(0.0, 0.0),
    m_turtleHeading(0.0),
    m_turtleColor(Qt::black),
    m_turtleHidden(false),
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

/**
 * @brief Return an image of the canvas.
 *
 * @return The canvas image.
 */
QImage TurtleCanvasGraphicsItem::toImage(bool transparentBackground,
                                         bool fitToUsedArea) const
{
    QMutexLocker lock(&m_mutex);
    QRect pixmapRect;
    QImage::Format imageFormat;

    pixmapRect  = fitToUsedArea ? m_usedRect : m_pixmap.rect();

    imageFormat = transparentBackground
                    ? QImage::Format_ARGB32_Premultiplied
                    : QImage::Format_RGB32;

    QImage image = QImage(pixmapRect.size(), imageFormat);

    QPainter painter(&image);
    if (transparentBackground)
    {
        painter.fillRect(image.rect(), Qt::transparent);
    }
    else
    {
        painter.fillRect(image.rect(), m_backgroundColor);
    }
    painter.drawPixmap(image.rect(), m_pixmap, pixmapRect);

    return image;
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
 * @brief Set the properties of the on-screen turtle.
 *
 * @param position The turtle's position on the screen.
 * @param heading The turtle's heading/direction.
 * @param color The color of the turtle.
 */
void TurtleCanvasGraphicsItem::setTurtle(const QPointF& position,
                                         qreal heading,
                                         const QColor& color)
{
    {
        QMutexLocker lock(&m_mutex);
        m_turtlePos     = position;
        m_turtleHeading = heading;
        m_turtleColor   = color;
    }

    emit canvasUpdated();
}

/**
 * @brief Gets the properties of the on-screen turtle.
 *
 * @param position The turtle's last set position.
 * @param heading The turtle's last set heading/direction.
 * @param color The turtle's last set color.
 */
void TurtleCanvasGraphicsItem::getTurtle(QPointF& position,
                                         qreal& heading,
                                         QColor& color) const
{
    QMutexLocker lock(&m_mutex);
    position = m_turtlePos;
    heading  = m_turtleHeading;
    color    = m_turtleColor;
}

/**
 * @brief Make the on-screen turtle visible.
 */
void TurtleCanvasGraphicsItem::showTurtle()
{
    {
        QMutexLocker lock(&m_mutex);
        m_turtleHidden = false;
    }

    emit canvasUpdated();
}

/**
 * @brief Hide the on-screen turtle.
 */
void TurtleCanvasGraphicsItem::hideTurtle()
{
    {
        QMutexLocker lock(&m_mutex);
        m_turtleHidden = true;
    }

    emit canvasUpdated();
}

/**
 * @brief Get whether or not the on-screen turtle is currently hidden.
 * @return
 */
bool TurtleCanvasGraphicsItem::turtleHidden()
{
    QMutexLocker lock(&m_mutex);
    return m_turtleHidden;
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
void TurtleCanvasGraphicsItem::drawLine(QLineF line, const QPen &pen)
{
    {
        QMutexLocker lock(&m_mutex);

        QPainter painter(&m_pixmap);
        painter.setRenderHint(QPainter::Antialiasing, m_antialiased);

        painter.setPen(pen);

        line.translate(static_cast<qreal>(m_pixmap.width())  / 2.0,
                       static_cast<qreal>(m_pixmap.height()) / 2.0);

        if (!m_antialiased)
        {
            // Rendering artifacts can occur when AA is disabled due to QPainter::drawLine's
            // apparent behaviour of casting the line's points from a qreal to an int without
            // rounding, which causes rendering artifacts where some lines are offset by 1 pixel.
            //
            // For example, if a coordinate value is 4.99999 then QPainter clips this to 4, which
            // causes an error.
            //
            // An example of a lua script which generates these artifacts is:
            //    for n=1,1000,1 do fd(n) rt(90) end
            //
            // which generates a square spiral. There should always be a 1px gap between each line,
            // but this is not always the case without this rounding fix.
            line = QLineF(std::round(line.x1()),
                          std::round(line.y1()),
                          std::round(line.x2()),
                          std::round(line.y2()));
        }

        // Translate the origin from the user's perspective (center of the drawing area)
        // to QPixmap's origin (top-left of the pixmap).
        painter.drawLine(line);

        updateUsedArea(line.p1());
        updateUsedArea(line.p2());
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

        // Map boundingBox from local coordinates to global coordinates.
        // Note that QMartrix::mapRect() returns the bounding rectangle
        // of the mapped rectangle (if rotations are applied), which is
        // exactly what we want.
        QMatrix worldMatrix = painter.worldMatrix();
        boundingBox = worldMatrix.mapRect(boundingBox);

        updateUsedArea(boundingBox.topLeft());
        updateUsedArea(boundingBox.topRight());
        updateUsedArea(boundingBox.bottomLeft());
        updateUsedArea(boundingBox.bottomRight());
    }

    emit canvasUpdated();
}

/**
 * @brief Get the canvas size.
 *
 * @return The canvas size.
 */
QSize TurtleCanvasGraphicsItem::size() const
{
    QMutexLocker lock(&m_mutex);
    return m_pixmap.size();
}

/**
 * @brief Change the canvas size.
 *
 * The canvasResized() signal is emitted after after the canvas is resized.
 *
 * @param newSize The size (in pixels) of the new canvas. If the size is the same
 *     as the current size then this method has no effect.
 */
void TurtleCanvasGraphicsItem::resize(QSize newSize)
{
    bool wasResized = false;

    {
        QMutexLocker lock(&m_mutex);

        const QSize oldSize = m_pixmap.size();

        if (newSize != oldSize)
        {
            prepareGeometryChange();

            QPixmap newpixmap(newSize);
            newpixmap.fill(Qt::transparent);

            QPainter painter(&newpixmap);

            int xoffset = (newSize.width() - oldSize.width()) / 2;
            int yoffset = (newSize.height() - oldSize.height()) / 2;
            painter.drawPixmap(QPoint(xoffset, yoffset),
                               m_pixmap,
                               m_pixmap.rect());

            m_pixmap = newpixmap;

            update();

            setPos(-static_cast<qreal>(newSize.width()) / 2.0,
                   -static_cast<qreal>(newSize.height()) / 2.0);

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
    static const QPointF turtlePoints[] =
    {
        QPointF(-10.0,  0.5),
        QPointF( 10.0,  0.5),
        QPointF(  0.0, -10.5)
    };

    QMutexLocker lock(&m_mutex);
    painter->fillRect(boundingRect(), m_backgroundColor);
    painter->drawPixmap(boundingRect(), m_pixmap, m_pixmap.rect());

    // Paint the turtle
    if (!m_turtleHidden)
    {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(static_cast<qreal>(m_pixmap.width())  / 2.0,
                           static_cast<qreal>(m_pixmap.height()) / 2.0);
        painter->translate(QPointF(m_turtlePos.x(),
                                   -m_turtlePos.y()));
        painter->rotate(m_turtleHeading);
        painter->setPen(m_turtleColor);

        painter->drawPolygon(&turtlePoints[0],
                             sizeof(turtlePoints)/sizeof(turtlePoints[0]));
    }
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

void TurtleCanvasGraphicsItem::updateUsedArea(const QPoint& point)
{
    const QRect rect = m_pixmap.rect();
    int top;
    int bottom;
    int left;
    int right;

    if (rect.contains(point))
    {
        top    = point.y();
        bottom = point.y();
        left   = point.x();
        right  = point.x();
    }
    else
    {
        top    = rect.top();
        bottom = rect.bottom();
        left   = rect.left();
        right  = rect.right();
    }

    if (point.x() < m_usedRect.left())
    {
        m_usedRect.setLeft(left);
    }
    if (point.x() > m_usedRect.right())
    {
        m_usedRect.setRight(right);
    }
    if (point.y() < m_usedRect.top())
    {
        m_usedRect.setTop(top);
    }
    if (point.y() > m_usedRect.bottom())
    {
        m_usedRect.setBottom(bottom);
    }

    assert(m_pixmap.rect().contains(point)
           ? m_usedRect.contains(point)
           : true);
    assert(m_pixmap.rect().contains(m_usedRect.topLeft()));
    assert(m_pixmap.rect().contains(m_usedRect.topRight()));
    assert(m_pixmap.rect().contains(m_usedRect.bottomLeft()));
    assert(m_pixmap.rect().contains(m_usedRect.bottomRight()));
}

void TurtleCanvasGraphicsItem::updateUsedArea(const QPointF& point)
{
    updateUsedArea(QPoint(static_cast<int>(std::round(point.x())),
                          static_cast<int>(std::round(point.y()))));
}
