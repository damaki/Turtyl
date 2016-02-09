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
 * @brief Manages real-time drawing & rendering of turtle graphics operations.
 *
 * The following methods in this class can be safely called by concurrently by a
 * background thread (i.e. the thread running the Lua scripts):
 *    * setBackgroundColor()
 *    * clear()
 *    * drawLine()
 *    * drawArc()
 *
 * Other methods can only be called by the UI thread.
 */
class TurtleGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    TurtleGraphicsItem();

    bool antialiased() const;
    void setAntialiased(bool on = true);

    void setBackgroundColor(const QColor& color);

    void clear();

    void drawLine(const QLineF& line, const QPen& pen);

    void drawArc(const QPointF& startPos,
                 qreal startAngle,
                 qreal angle,
                 qreal radius,
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