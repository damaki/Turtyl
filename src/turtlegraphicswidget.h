#ifndef TURTLEGRAPHICSWIDGET_H
#define TURTLEGRAPHICSWIDGET_H

#include <QMutex>
#include <QWidget>
#include <QPixmap>

class TurtleGraphicsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TurtleGraphicsWidget(QWidget *parent = 0);

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

signals:
    void canvasUpdated();

public slots:

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    mutable QMutex m_mutex;

    QPixmap m_pixmap;
    QColor m_backgroundColor;

    bool m_antialiased;
};

#endif // TURTLEGRAPHICSWIDGET_H
