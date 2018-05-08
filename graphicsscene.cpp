#include "graphicsscene.h"
#include <QDebug>
#include <math.h>
extern int pix_per_brick;
extern int deletemode;
// TODO:
// delete line
// right angle turnning instead of sloped line
graphicsscene::graphicsscene(QObject *parent) :QGraphicsScene(parent)
{

}


// Start drawing line
void graphicsscene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

    if(!pressed){                                             // start a new turnning line
        int roundx1 = (int)mouseEvent->scenePos().x() % pix_per_brick;
        int roundy1 = (int)mouseEvent->scenePos().y() % pix_per_brick;

        line *drawline = new line();
        drawline->x[0] = mouseEvent->scenePos().x() - roundx1;
        drawline->y[0] = mouseEvent->scenePos().y() - roundy1;
        drawline->x[1] = drawline->x[0];
        drawline->y[1] = drawline->y[0];
        segline = drawline;                                                 // the first segment
        turnline = new line();
        pressed = true;
        qDebug() << "Start a new line";
//        qDebug() << "press" << drawline->x[turnline->segments] << drawline->x[turnline->segments] << drawline->y[turnline->segments+1] << drawline->y[turnline->segments+1] << pressed;
    }
    else if(pressed){
        if(turnline->segments % 2 != 0){
            this->addItem(segline2);
            qDebug() << "Add a segment" << turnline->segments << "||" << segline2->x[0] << segline2->y[0] << segline2->x[1] << segline2->y[1];
            //update data to turnline
            turnline->x[turnline->segments] = segline2->x[0];
            turnline->y[turnline->segments] = segline2->y[0];
            turnline->x[turnline->segments+1] = segline2->x[1];
            turnline->y[turnline->segments+1] = segline2->y[1];
            turnline->segments++;

        // start a new segnemt
            line *drawline = new line();
            drawline->x[0] = turnline->x[turnline->segments];
            drawline->y[0] = turnline->y[turnline->segments];
            drawline->x[1] = turnline->x[turnline->segments];
            drawline->y[1] = turnline->y[turnline->segments];
            segline = drawline;
        }
        else{
            this->addItem(segline);
            qDebug() << "Add a segment" << turnline->segments << "||" << segline->x[0] << segline->y[0] << segline->x[1] << segline->y[1];
            //update data to turnline
            turnline->x[turnline->segments] = segline->x[0];
            turnline->y[turnline->segments] = segline->y[0];
            turnline->x[turnline->segments+1] = segline->x[1];
            turnline->y[turnline->segments+1] = segline->y[1];
            turnline->segments++;

            line *drawline = new line();
            drawline->x[0] = turnline->x[turnline->segments];
            drawline->y[0] = turnline->y[turnline->segments];
            drawline->x[1] = turnline->x[turnline->segments];
            drawline->y[1] = turnline->y[turnline->segments];
            segline2 = drawline;
        }
    }

}

// Move the line with the cursor while drawing line
void graphicsscene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(pressed){
        if(turnline->segments % 2 == 0){
            if(abs((mouseEvent->scenePos().x() - segline->x[0])) < 0.1){
                segline->x[1] = segline->x[0];
                segline->y[1] = mouseEvent->scenePos().y();
            }
            else{
                slope = (mouseEvent->scenePos().y() - segline->y[0]) / (mouseEvent->scenePos().x() - segline->x[0]);
                if(slope > 1 || slope < -1){
                    segline->x[1] = segline->x[0];
                    segline->y[1] = mouseEvent->scenePos().y();
                }
                else{
                    segline->x[1] = mouseEvent->scenePos().x();
                    segline->y[1] = segline->y[0];
                }
            }
            qDebug() << "A" << segline->x[0] << segline->y[0] << segline->x[1] << segline->y[1];
        }
        else{
            if(abs((mouseEvent->scenePos().x() - segline2->x[0])) < 0.1){
                segline2->x[1] = segline2->x[0];
                segline2->y[1] = mouseEvent->scenePos().y();
            }
            else{
                slope = (mouseEvent->scenePos().y() - segline2->y[0]) / (mouseEvent->scenePos().x() - segline2->x[0]);
                if(slope > 1 || slope < -1){
                    segline2->x[1] = segline2->x[0];
                    segline2->y[1] = mouseEvent->scenePos().y();
                }
                else{
                    segline2->x[1] = mouseEvent->scenePos().x();
                    segline2->y[1] = segline2->y[0];
                }
            }
            qDebug() << "B" << segline2->x[0] << segline2->y[0] << segline2->x[1] << segline2->y[1];
        }

    }
}

// Draw the line following the grid
void graphicsscene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{

//        int roundx2 = (int)turnline->x[1] % pix_per_brick;
//        int roundy2 = (int)turnline->y[1] % pix_per_brick;
//        QLineF *newline = new QLineF(myline->line().p1().x(), myline->line().p1().y(), (myline->line().p2().x() - roundx2), (myline->line().p2().y() - roundy2));
//        myline->setLine(*newline);
//        alllines.prepend(newline);

//        this->addItem(turnline);
//        pressed = false;
//
//        if(deletemode){
//            alllines.removeOne()
//        }
}

void graphicsscene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent){

//    this->addItem(segline);
//    if(turnline->segments % 2 != 0){
//        this->addItem(segline2);
//        qDebug() << "Add a segment" << turnline->segments << "||" << segline2->x[0] << segline2->y[0] << segline2->x[1] << segline2->y[1];
//        //update data to turnline
//        turnline->x[turnline->segments] = segline2->x[0];
//        turnline->y[turnline->segments] = segline2->y[0];
//        turnline->x[turnline->segments+1] = segline2->x[1];
//        turnline->y[turnline->segments+1] = segline2->y[1];
//        turnline->segments++;

//        qDebug() << "segment" << turnline->segments;
//    }
//    else{
//        this->addItem(segline);
//        qDebug() << "Add a segment" << turnline->segments << "||" << segline->x[0] << segline->y[0] << segline->x[1] << segline->y[1];
//        //update data to turnline
//        turnline->x[turnline->segments] = segline->x[0];
//        turnline->y[turnline->segments] = segline->y[0];
//        turnline->x[turnline->segments+1] = segline->x[1];
//        turnline->y[turnline->segments+1] = segline->y[1];
//        turnline->segments++;

//        qDebug() << "segment" << turnline->segments;
//    }
    alllines.prepend(turnline);
    pressed = false;
    qDebug() << "End a line";
//    qDebug() << "double" << turnline->x[turnline->segments] << turnline->x[turnline->segments+1] << turnline->y[turnline->segments] << turnline->y[turnline->segments+1] << pressed;
}
