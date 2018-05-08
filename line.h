#ifndef LINE_H
#define LINE_H
#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsItem>
#include <QPoint>
#include <QWidget>
#include <QMenu>
#include <QAction>
class line : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    line();
    QRectF boundingRect() const;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);


    float x[10];
    float y[10];
    int segments = 0;
protected:
//    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // LINE_H
