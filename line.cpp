#include "line.h"
extern bool deletemode;
line::line()
{
//    setFlag(ItemIsMovable);
}

QRectF line::boundingRect() const
{
    // outer most edges
    int slope = (y[1] - y[0]) / (x[1] - x[0]);
    if(slope > 1 || slope < -1){
        return QRectF(x[0], y[0], 3, y[1] - y[0]);
    }
    else{
        return QRectF(x[0], y[0], x[1] - x[0], 3);
    }

}

void line::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();
    QBrush brush(Qt::gray);
    painter->setBrush(brush);
    QPen pen(Qt::black, 1);
    painter->setPen(pen);
    painter->drawRect(rect);
    update();

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

////void unit::mousePressEvent(QGraphicsSceneMouseEvent *event)
////{
////    Pressed = true;
////    update();
////    QGraphicsItem::mousePressEvent(event);
////}

//void line::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//    QGraphicsItem::mouseReleaseEvent(event);
////    if(deletemode){
////        emit delete_this_item(this);
////    }
//}




