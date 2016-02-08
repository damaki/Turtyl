#include "turtlegraphicswidget.h"
#include <QMutexLocker>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <cassert>

static const int DEFAULT_SIZE = 2048;

TurtleGraphicsWidget::TurtleGraphicsWidget(QWidget *parent) :
    QWidget(parent),
    m_mutex(),
    m_pixmap(DEFAULT_SIZE, DEFAULT_SIZE),
    m_backgroundColor(Qt::black),
    m_antialiased(false)
{
    connect(this, SIGNAL(canvasUpdated()),
            this, SLOT(update()),
            Qt::QueuedConnection);

    setFixedSize(DEFAULT_SIZE, DEFAULT_SIZE);

    m_pixmap.fill(Qt::transparent);
}

bool TurtleGraphicsWidget::antialiased() const
{
    QMutexLocker lock(&m_mutex);
    return m_antialiased;
}

void TurtleGraphicsWidget::setAntialiased(const bool on)
{
    QMutexLocker lock(&m_mutex);
    m_antialiased = on;
}

void TurtleGraphicsWidget::setBackgroundColor(const QColor& color)
{
    QMutexLocker lock(&m_mutex);
    m_backgroundColor = color;

    emit canvasUpdated();
}

void TurtleGraphicsWidget::clear()
{
    QMutexLocker lock(&m_mutex);
    m_pixmap.fill(Qt::transparent);

    emit canvasUpdated();
}

void TurtleGraphicsWidget::drawLine(const QLineF &line, const QPen &pen)
{
    QMutexLocker lock(&m_mutex);

    QPainter painter(&m_pixmap);
    painter.setRenderHint(QPainter::Antialiasing, m_antialiased);

    painter.setPen(pen);

    // Translate the origin from the user's perspective (center of the drawing area)
    // to QPixmap's origin (top-left of the pixmap).
    painter.drawLine(line.translated(static_cast<qreal>(m_pixmap.width())  / 2.0,
                                       static_cast<qreal>(m_pixmap.height()) / 2.0));

    emit canvasUpdated();
}

void TurtleGraphicsWidget::drawArc(const QPointF &startPos,
                                   qreal startAngle,
                                   qreal angle,
                                   qreal radius,
                                   const QPen &pen)
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

    emit canvasUpdated();
}

void TurtleGraphicsWidget::resizeEvent(QResizeEvent *event)
{
    event->accept();

    QMutexLocker lock(&m_mutex);

    // For now, the canvas is always square.
    assert(event->size().width() == event->size().height());

    int oldSize = event->oldSize().width();
    int newSize = event->size().width();

    if (newSize != oldSize)
    {
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
    }
}

void TurtleGraphicsWidget::paintEvent(QPaintEvent *event)
{
    event->accept();

    QMutexLocker lock(&m_mutex);
    QPainter painter(this);
    painter.fillRect(event->rect(), m_backgroundColor);
    painter.drawPixmap(event->rect(), m_pixmap, event->rect());
}
