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
#ifndef TURTLEGRAPHICSWIDGET_H
#define TURTLEGRAPHICSWIDGET_H

#include <QMutex>
#include <QGraphicsItem>
#include <QPixmap>

/**
 * @brief Canvas for real-time drawing & rendering of turtle graphics.
 *
 * The following methods in this class can be safely called by concurrently by a
 * background thread (i.e. the thread running the Lua scripts):
 *    * setBackgroundColor()
 *    * clear()
 *    * drawLine()
 *    * drawArc()
 *
 * Other methods can only be called by the UI thread.
 *
 * @section Coordinate System
 *
 * The coordinate system used by this class is different to the rest of the Qt framework.
 * In this class, the origin (0,0) is at the center of the canvas, with the X coordinate
 * increasing as the pen moves @em right on the canvas, and the Y coordinate increasing
 * as the pen moves @em up the canvas. The (x,y) coordinates on an example 10x10 canvas
 * is shown below:
 *
 * @code
 *             width = 10
 * |<------------------------------->|
 *
 *  (-5,5)           (0,5)            (5,5)
 * +----------------+----------------+      ---
 * |                |                |       ^
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |(-5,0)          |(0,0)           |(5,0)  |
 * +----------------+----------------+       | height = 10
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |                |                |       |
 * |(-5,-5)         |(0,-5)          |(5,-5) v
 * +----------------+----------------+      ---
 * @endcode
 *
 * All drawing operations (e.g. drawLine()) use the above coordinate system.
 *
 * @subsection Resizing the canvas
 * The canvas size can be changed at any time by calling resize(), which will
 * change the width and height of the canvas to the new size (note that the
 * canvas is always square).
 *
 * When the canvas is resized its width and height are updated relative to the
 * origin at (0,0). If the canvas size is reduced then drawings at the edge of
 * the canvas are erased. The drawings at the origin are unaffected.
 *
 * Similarly, if the canvas size is increased then extra space is added at the
 * edges of the canvas.
 */
class TurtleCanvasGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    TurtleCanvasGraphicsItem();

    QImage toImage(bool flatten) const;

    bool antialiased() const;
    void setAntialiased(bool on = true);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    void clear();

    void drawLine(QLineF line, const QPen& pen);

    void drawArc(const QPointF& centerPos,
                 qreal startAngle,
                 qreal angle,
                 qreal xradius,
                 qreal yradius,
                 const QPen& pen);

    int size() const;
    void resize(int newSize);

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void canvasUpdated();
    void canvasResized();

private slots:
    void callUpdate();

private:
    mutable QMutex m_mutex;

    QPixmap m_pixmap;
    QColor m_backgroundColor;

    bool m_antialiased;
};

#endif // TURTLEGRAPHICSWIDGET_H
