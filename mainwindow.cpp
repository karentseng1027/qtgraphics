#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "switch.h"
#include "svgreader.h"
//#include "save_yn_dialog.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsItem>
#include <QPoint>
#include <QMainWindow>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QPrinter>
/////////////////////////////////////  CHIP INFO  /////////////////////////////////////
bool deletemode = false;
bool linemode = false;
bool detailmode = false;

//setting parameter
int chip_length_cm = 3;
int chip_width_cm = 2;
int chip_border_mm = 3;

int cp_length_mm = 2;
int cp_width_mm = 4;

int de1_length_mm = 1;
int de1_width_mm = 5;
int de2_length_mm = 3;
int de2_width_mm = 3;

int de_spacing_um = 500;
int cp_spacing_um = 300;
int line_width_mm = 3;

//int dispenser_mm = 10;
int pix_per_brick = 20;

//unit color
QColor merge_color = QColor(255, 208, 166, 127);
QColor cycling_color = QColor(1, 96, 177, 127);
QColor moving_color = QColor(255, 184, 184, 127);
QColor dispenser_color = QColor(215, 230, 144, 127);
QColor heat_color = QColor(108, 137, 147, 127);

//Pen setting
QPen graypen(Qt::gray);
QPen redpen(Qt::red);
QPen blackpen(Qt::black);
QPen whitepen(Qt::white);

//Brush setting
QBrush nullitem(QColor(94, 94, 94, 54));
//
int border_px;
int brick_xnum;
int brick_ynum;
int chip_width_px;
int chip_height_px;
int brick_x_start;
int brick_y_start;

/////////////////////////////////////  MAINWINDOW  /////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
    setAcceptDrops(true);
    ui->setupUi(this);
    this->linescene = this->CreateLineScene();
    this->mainscene = this->CreateNewScene();

    //TOOGLE BUTTON
    SwitchControl *pSwitchControl = new SwitchControl(this->centralWidget());
    pSwitchControl->setFixedWidth(55);
    pSwitchControl->setFixedHeight(28);
    pSwitchControl->move(365, 633);
    mainscene->addWidget(pSwitchControl);
    pSwitchControl->setToggle(false);
    connect(pSwitchControl, SIGNAL(toggled(bool)), this, SLOT(mode_label(bool)));

    //NEW ACTION
    QAction *New = ui->actionNew;
    New->setShortcuts(QKeySequence::New);
    connect(New, SIGNAL(triggered()), this, SLOT(new_chip_clicked()));

    //SAVE ACTION
    QAction *Save = ui->actionSave;
    Save->setShortcuts(QKeySequence::Save);
    connect(Save, SIGNAL(triggered()), this, SLOT(save_svg()));

    //LOAD ACTION
    QAction *Load = ui->actionOpen;
    Load->setShortcuts(QKeySequence::Open);
    connect(Load, SIGNAL(triggered()), this, SLOT(load_svg_clicked()));

    //EXPORT ACTION
    QAction *Export = ui->actionExport;
    Export->setShortcuts(QKeySequence::Italic);
    connect(Export, SIGNAL(triggered()), this, SLOT(export_clicked()));

    //CLOSE ACTION
    QAction *Close = ui->actionClose;
    Close->setShortcut(QKeySequence::Close);
    connect(Close, SIGNAL(triggered()), this, SLOT(close_clicked()));

    //EXPORT PDF ACTION
    QAction *Pdf = ui->actionPdf;
//    Pdf->setShortcuts(QKeySequence::);
    connect(Pdf, SIGNAL(triggered()), this, SLOT(pdf_clicked()));

    ui->view->setScene(mainscene);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//Create New Scene
QGraphicsScene* MainWindow::CreateNewScene(){
    QGraphicsScene * scene = new QGraphicsScene(0, 0, 600, 400, ui->view);
//    graphicsscene* scene = new graphicsscene(ui->view);               //create a new scene
//    scene->setSceneRect(QRectF(0, 0, 600, 400));
    scene->setBackgroundBrush(Qt::white);

    pix_per_brick = fmin(600/((10*chip_length_cm*1000/de_spacing_um)),                  //calculate the size of each brick that fits the screeen the most
                         400/((10*chip_width_cm*1000/de_spacing_um)));

        //1. outer border(nothing to do with real chip size, just the canvas size)
        scene->addLine(0, 0, scene->width(), 0, graypen);
        scene->addLine(0, scene->height(), scene->width(), scene->height(), graypen);
        scene->addLine(0, 0, 0, scene->height(), graypen);
        scene->addLine(scene->width(), 0, scene->width(), scene->height(), graypen);

        border_px = chip_border_mm*1000/de_spacing_um*pix_per_brick;                        //border width in pixel
        brick_xnum = (10*chip_length_cm - 2*chip_border_mm)*1000/de_spacing_um;             //how many background dots are needed in x-azis
        brick_ynum = (10*chip_width_cm - 2*chip_border_mm)*1000/de_spacing_um;              //how many background dots are needed in y-axis
        chip_height_px = brick_ynum*pix_per_brick+border_px*2;                              //chip_length in pixel
        chip_width_px = brick_xnum*pix_per_brick+border_px*2;                               //chip_width in pixel


        brick_x_start = (600 - chip_width_px)/2 + border_px;                                //where to start drawing dots in y-axis
        brick_y_start = (400 - chip_height_px)/2 + border_px;

        //background grid (in dotted form)
        for(int i = 0; i <= brick_xnum; i++){
            for(int j = 0; j <= brick_ynum; j++){
                scene->addEllipse(brick_x_start+i*pix_per_brick, brick_y_start+j*pix_per_brick, 1, 1, graypen);
            }
        }

        //chip scale                                                                        //show dots of scale near the chip border, each interval stands for 1cm in real size
        int cm_to_px = 10000/de_spacing_um*pix_per_brick;                                   //how many pixels are in 1 cm
        for(int i = 1; i < chip_length_cm; i++){
            scene->addEllipse(brick_x_start-border_px + i*cm_to_px, brick_y_start-border_px+chip_height_px-2, 1, 1, redpen);
//            QString j = QString::number(i);
//            QGraphicsTextItem *text = mainscene->addText(j + "cm");
//            text->setPos(400, 400);
        }
        for(int i = 1; i < chip_width_cm; i++){
            scene->addEllipse(brick_x_start-border_px+2, brick_y_start-border_px + i*cm_to_px, 1, 1, redpen);
        }

        //2. pix_per_brick (save and load)                                                  //show this to save data, prepared for the next LOAD
        scene->addRect(0, 10, pix_per_brick, 10, whitepen);

        //3. border_px (save and load)
        scene->addRect(0, 20, border_px, 10, whitepen);                                               //show this to save data, prepared for the next LOAD

        //4. chip border
        scene->addRect(brick_x_start-border_px, brick_y_start-border_px, chip_width_px, chip_height_px, redpen);    //the exact border of the chip

        //5. scale item                                                                     //show a 1cm scale
        scene->addRect(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px , 460, cm_to_px, 2, redpen);
        QGraphicsTextItem *text = scene->addText("1cm");
        text->setPos(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px + (cm_to_px/2) - 12, 463);

        return scene;
}

graphicsscene* MainWindow::CreateLineScene(){
    graphicsscene* scene = new graphicsscene(ui->view);                                 //create a new scene
    scene->setSceneRect(QRectF(0, 0, 600, 400));
    scene->setBackgroundBrush(Qt::white);
    pix_per_brick = fmin(600/((10*chip_length_cm*1000/de_spacing_um)),                  //calculate the size of each brick that fits the screeen the most
                         400/((10*chip_width_cm*1000/de_spacing_um)));

        //1. outer border(nothing to do with real chip size, just the canvas size)
        scene->addLine(0, 0, scene->width(), 0, graypen);
        scene->addLine(0, scene->height(), scene->width(), scene->height(), graypen);
        scene->addLine(0, 0, 0, scene->height(), graypen);
        scene->addLine(scene->width(), 0, scene->width(), scene->height(), graypen);

        border_px = chip_border_mm*1000/de_spacing_um*pix_per_brick;                        //border width in pixel
        brick_xnum = (10*chip_length_cm - 2*chip_border_mm)*1000/de_spacing_um;             //how many background dots are needed in x-azis
        brick_ynum = (10*chip_width_cm - 2*chip_border_mm)*1000/de_spacing_um;              //how many background dots are needed in y-axis
        chip_height_px = brick_ynum*pix_per_brick+border_px*2;                              //chip_length in pixel
        chip_width_px = brick_xnum*pix_per_brick+border_px*2;                               //chip_width in pixel


        brick_x_start = (600 - chip_width_px)/2 + border_px;                                //where to start drawing dots in y-axis
        brick_y_start = (400 - chip_height_px)/2 + border_px;

        //background grid (in dotted form)
        for(int i = 0; i <= brick_xnum; i++){
            for(int j = 0; j <= brick_ynum; j++){
                scene->addEllipse(brick_x_start+i*pix_per_brick, brick_y_start+j*pix_per_brick, 1, 1, graypen);
            }
        }

        //chip scale                                                                        //show dots of scale near the chip border, each interval stands for 1cm in real size
        int cm_to_px = 10000/de_spacing_um*pix_per_brick;                                   //how many pixels are in 1 cm
        for(int i = 1; i < chip_length_cm; i++){
            scene->addEllipse(brick_x_start-border_px + i*cm_to_px, brick_y_start-border_px+chip_height_px-2, 1, 1, redpen);
        }
        for(int i = 1; i < chip_width_cm; i++){
            scene->addEllipse(brick_x_start-border_px+2, brick_y_start-border_px + i*cm_to_px, 1, 1, redpen);
        }

        //2. pix_per_brick (save and load)                                                  //show this to save data, prepared for the next LOAD
        scene->addRect(0, 10, pix_per_brick, 10, whitepen);

        //3. border_px (save and load)
        scene->addRect(0, 20, border_px, 10, whitepen);                                               //show this to save data, prepared for the next LOAD

        //4. chip border
        scene->addRect(brick_x_start-border_px, brick_y_start-border_px, chip_width_px, chip_height_px, redpen);    //the exact border of the chip

        //5. scale item                                                                     //show a 1cm scale
        scene->addRect(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px , 460, cm_to_px, 2, redpen);
        QGraphicsTextItem *text = scene->addText("1cm");
        text->setPos(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px + (cm_to_px/2) - 12, 463);

        return scene;
}

//MODE LABEL                                                                            //shows the mode its in
void MainWindow::mode_label(bool bChecked){

    if(bChecked){
        for(unit *item : allunits){
            linescene->addRect(item->xi*pix_per_brick, item->yi*pix_per_brick, item->length*pix_per_brick, item->width*pix_per_brick, redpen, nullitem);
        }
        ui->view->setScene(linescene);
        linemode = true;
        ui->mode_label->setText("CUSTOMIZED");
        QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/cursor.png");
        QCursor pencil = QCursor(*e, -10, -10);
        ui->view->setCursor(pencil);
    }
    else{
        for(line *turnline : linescene->alllines){
//            qDebug() << turnline->x[0] << turnline->y[0] << turnline->x[1] << turnline->y[1];
//            mainscene->addLine(line->x1(), line->y1(), line->x2(), line->y2(), graypen);
        }
        qDebug() << "line size : "<< linescene->alllines.size();
        ui->view->setScene(mainscene);
        linemode = false;
        ui->mode_label->setText("SIMPLE");
        ui->view->setCursor(Qt::ArrowCursor);
    }
}

//////////////////////////////////
//////  ACTIONS  /////////////////////////////////////
void MainWindow::new_chip_clicked()
{
    this->save_svg();
    QGraphicsScene *newscene = CreateNewScene();
    delete(ui->view->scene());
    allunits.clear();
    this->mainscene = newscene;
    ui->view->setScene(newscene);
}

//SAVE SVG
void MainWindow::save_svg()
{
    //Take file path and name that will create
    QString newPath = QFileDialog::getSaveFileName(this, "Save Chip .SVG", path, tr("SVG files (*.svg)"));
    if(newPath.isEmpty()) return;
    path = newPath;

    QSvgGenerator generator;                                                            //Create a file generator object
    generator.setFileName(path);                                                        //We set the path to the file where to save vector graphics
    generator.setSize(QSize(mainscene->width(), mainscene->height()));                  //Set the dimensions of the working area of the document in millimeters
    generator.setViewBox(QRect(0, 0, mainscene->width(), mainscene->height()));         //Set the work area in the coordinates
    generator.setTitle("Drag");                                                         //The title document

    QPainter painter;
    painter.begin(&generator);
    mainscene->render(&painter);
    painter.end();

    //At the end we get a vector drawing file with the contents of the graphic scenes
}

//LOAD SVG
void MainWindow::load_svg_clicked()
{
    QString newPath = QFileDialog::getOpenFileName(this, "Open SVG", path, tr("SVG files (*.svg)"));
    if(newPath.isEmpty())return;
    path = newPath;

    allunits.clear();
    mainscene->clear();
    mainscene->setSceneRect(SvgReader::getSizes(path));

    int flag = 4;

    foreach (unit *item, SvgReader::getElements(path)) {
        unit *rect = item;
        if(flag==4){
            qDebug() << "outer box";
            //1. outer border
            mainscene->addLine(0, 0, mainscene->width(), 0, graypen);
            mainscene->addLine(0, mainscene->height(), mainscene->width(), mainscene->height(), graypen);
            mainscene->addLine(0, 0, 0, mainscene->height(), graypen);
            mainscene->addLine(mainscene->width(), 0, mainscene->width(), mainscene->height(), graypen);
            flag--;
        }else if(flag==3){
            pix_per_brick = rect->length;
            //2. pix_per_brick
            mainscene->addRect(0, 10, pix_per_brick, 10, whitepen);
            qDebug() << "pix_per_brick : "<< pix_per_brick;
            flag--;
        }else if(flag==2){
            border_px = rect->length;
            //3. border_px
            mainscene->addRect(0, 20, border_px, 10, whitepen);
            qDebug() << "border_px : " << border_px;
            flag--;
        }else if(flag==1){
            brick_xnum = (rect->length - 2*border_px) / pix_per_brick;
            brick_ynum = (rect->width - 2*border_px) / pix_per_brick;
            chip_width_px = rect->length;
            chip_height_px = rect->width;

            brick_x_start = (600 - chip_width_px)/2 + border_px;
            brick_y_start = (400 - chip_height_px)/2 + border_px;
            //background grid
            for(int i = 0; i <= brick_xnum; i++){
                for(int j = 0; j <= brick_ynum; j++){
                    mainscene->addEllipse(brick_x_start+i*pix_per_brick, brick_y_start+j*pix_per_brick, 1, 1, graypen);
                }
            }

            //chip scale
            int cm_to_px = 10000/de_spacing_um*pix_per_brick;
            for(int i = 1; i < chip_length_cm; i++){
                mainscene->addEllipse(brick_x_start-border_px + i*cm_to_px, brick_y_start-border_px+chip_height_px-2, 1, 1, redpen);
            }
            for(int i = 1; i < chip_width_cm; i++){
                mainscene->addEllipse(brick_x_start-border_px+2, brick_y_start-border_px + i*cm_to_px, 1, 1, redpen);
            }
            //4. border
            mainscene->addRect(brick_x_start-border_px, brick_y_start-border_px , rect->length, rect->width, redpen);
            flag--;
        }else{
            //reload all units that were created
            rect->xi /= pix_per_brick;
            rect->yi /= pix_per_brick;
            rect->length /= pix_per_brick;
            rect->width /= pix_per_brick;
            mainscene->addItem(rect);
            allunits.prepend(rect);
            connect(rect, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
        }
    }
    //5. scale item
    int cm_to_px = 10000/de_spacing_um*pix_per_brick;
    mainscene->addRect(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px , 460, cm_to_px, 2, redpen);
    QGraphicsTextItem *text = mainscene->addText("1cm");
    text->setPos(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px + (cm_to_px/2) - 10, 463);
}

//EXPORT AI
void MainWindow::export_clicked()
{
    QString newPath = QFileDialog::getSaveFileName(this, "Save Ai", path, tr("AI files (*.ai)"));
    if (newPath.isEmpty()) return;
    path = newPath;

    QPrinter printer( QPrinter::HighResolution );
    printer.setPageSize( QPrinter::A4 );
    printer.setOrientation( QPrinter::Portrait );
    printer.setOutputFormat( QPrinter::NativeFormat );
    printer.setOutputFileName(path); // file will be created in your build directory (where debug/release directories are)

    QPainter p;

       if( !p.begin( &printer ) )
       {
           qDebug() << "Error!";
           return;
       }
        // reset all sizes to REAL SIZE
       QGraphicsScene *export_scene = new QGraphicsScene(0, 0, 9921, 14031);
       float px_to_cm = 9921/21;

       // border in REAL SIZE
       export_scene->addRect(0, 0, chip_length_cm*px_to_cm, chip_width_cm*px_to_cm, redpen);

       //units in REAL SIZE
       for(unit *item : allunits){
            unit *export_item = new unit();
            //export_item = item;
            export_item->xi = (item->xi - (brick_x_start-border_px)/pix_per_brick) * de_spacing_um* px_to_cm  / 10000 / pix_per_brick;
            export_item->yi = item->yi * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->length = item->length * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->width = item->width *de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            if(item->type == "merge"){
                export_item->color = merge_color;
            }else if(item->type == "dispenser"){
                export_item->color = dispenser_color;
            }else if(item->type == "move"){
                export_item->color = moving_color;
            }else if(item->type == "cycle"){
                export_item->color = cycling_color;
            }else{
                export_item->color = heat_color;
            }
            export_scene->addItem(export_item);
            qDebug() << "brick_x_start" << brick_x_start;
            qDebug() << "border_px" << border_px;
            qDebug() << "pix_per_brick" << pix_per_brick;
            qDebug() << "xi" << export_item->xi*pix_per_brick;
            qDebug() << "yi" << export_item->yi*pix_per_brick;
            qDebug() << "length" << export_item->length * pix_per_brick;
            qDebug() << "width" << export_item->width * pix_per_brick;
       }
       export_scene->render(&p);
       p.end();
}
//EXPORT PDF
void MainWindow::pdf_clicked()
{
    QString newPath = QFileDialog::getSaveFileName(this, "Save Pdf", path, tr("PDF files (*.pdf)"));
    if (newPath.isEmpty()) return;
    path = newPath;

    QPrinter printer( QPrinter::HighResolution );
    printer.setPageSize( QPrinter::A4 );
    printer.setOrientation( QPrinter::Portrait );
    printer.setOutputFormat( QPrinter::NativeFormat );
    printer.setOutputFileName(path); // file will be created in your build directory (where debug/release directories are)

    QPainter p;

       if( !p.begin( &printer ) )
       {
           qDebug() << "Error!";
           return;
       }
        // reset all sizes to REAL SIZE
       QGraphicsScene *export_scene = new QGraphicsScene(0, 0, 9921, 14031);
       float px_to_cm = 9921/21;

       // border in REAL SIZE
       export_scene->addRect(0, 0, chip_length_cm*px_to_cm, chip_width_cm*px_to_cm, redpen);

       //units in REAL SIZE
       for(unit *item : allunits){
            unit *export_item = new unit();
            //export_item = item;
            export_item->xi = (item->xi - (brick_x_start-border_px)/pix_per_brick) * de_spacing_um* px_to_cm  / 10000 / pix_per_brick;
            export_item->yi = item->yi * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->length = item->length * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->width = item->width *de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            if(item->type == "merge"){
                export_item->color = merge_color;
            }else if(item->type == "dispenser"){
                export_item->color = dispenser_color;
            }else if(item->type == "move"){
                export_item->color = moving_color;
            }else if(item->type == "cycle"){
                export_item->color = cycling_color;
            }else{
                export_item->color = heat_color;
            }
            export_scene->addItem(export_item);
            qDebug() << "brick_x_start" << brick_x_start;
            qDebug() << "border_px" << border_px;
            qDebug() << "pix_per_brick" << pix_per_brick;
            qDebug() << "xi" << export_item->xi*pix_per_brick;
            qDebug() << "yi" << export_item->yi*pix_per_brick;
            qDebug() << "length" << export_item->length * pix_per_brick;
            qDebug() << "width" << export_item->width * pix_per_brick;
       }
       export_scene->render(&p);
       p.end();
}

// CLOSE
void MainWindow::close_clicked()
{
    save_yn_dialog *want = new save_yn_dialog(this);
    want->setWindowTitle("Do you want to save this chip?");
    connect(want, SIGNAL(yes_save(save_yn_dialog *)), this, SLOT(close_window(save_yn_dialog *)));
//    connect(want, SIGNAL())
    want->show();
}

void MainWindow::close_window(save_yn_dialog *s)
{
    if(s->want_to_save){
        this->save_svg();
    }
    this->close();
}

/////////////////////////////////////  TOOL  BAR  /////////////////////////////////////
//DELETE (ERASER)
void MainWindow::on_eraser_clicked()
{
    deletemode = 1-deletemode;
    qDebug() << "number : " << allunits.count();
    qDebug() << "GLOBAL MODE : " <<deletemode;


    //show different cursor while deleting
//    QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/eraser_cursor.png");
//    QPixmap e(":/MainWindow/Icons/Icons/eraser_cursor.png");
//    QCursor cursorErase = QCursor(QPixmap(":/MainWindow/Icons/Icons/eraser_cursor.png"),0 , 0);
//    QCursor cursorErase = QCursor(*e ,0 , 0);
    if(deletemode == true){
        QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/eraser_cursor.png");
        QCursor cursorErase = QCursor(*e,0 , 0);
        ui->eraser->setStyleSheet("background-color: rgb(64, 72, 91);");
        ui->view->setCursor(cursorErase);
    } else {
        ui->eraser->setStyleSheet("background-color: rgb(42, 48, 58);");
        ui->view->setCursor(Qt::ArrowCursor);
    }
}

//remove the deleted item from allunits list
void MainWindow::delete_from_list(unit *item)
{
    allunits.removeOne(item);
    delete item;
}

void MainWindow::on_controlpad_btn_clicked()
{
    int size = allunits.size();
    int mm_to_px = 1000/de_spacing_um*pix_per_brick;
    int cp_start = brick_x_start;

    for(int i=0; i<size; i++){
        mainscene->addRect(cp_start, brick_y_start, cp_length_mm*mm_to_px, cp_width_mm*mm_to_px, graypen, QColor(94, 93, 93, 54));
        cp_start += cp_length_mm*mm_to_px + cp_spacing_um * mm_to_px / 1000;
    }
}
/////////////////////////////////////  CREATE COMPONENTS  /////////////////////////////////////
//MOVE
void MainWindow::on_move_create_clicked()
{
    int number = ui->move_num->value();
    int position = 5;
    while(number--){
        unit *move = new unit();
        move->type = "move";
        move->xi = position;
        move->yi = 0;
        move->de_type = ui->move_num->value();
        move->de_xnum = ui->move_size->text().toInt();
        move->de_ynum = 1;
        move->length = move->de_xnum*de1_length_mm*1000/de_spacing_um + move->de_xnum-1;
        move->width = move->de_ynum*de1_width_mm*1000/de_spacing_um;
        move->color = moving_color;
        ui->view->scene()->addItem(move);
        allunits.prepend(move);
        position += 5;
        connect(move, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}


//DISPENSER
void MainWindow::on_dispenser_create_clicked()
{
    int number = ui->dispenser_num->value();
    int position = 5;
    while(number--){
        unit *dispenser = new unit();
        dispenser->type = "dispenser";
        dispenser->xi = position;
        dispenser->yi = 5;
        dispenser->de_type = 0;
        dispenser->de_xnum = 1;
        dispenser->de_ynum = 1;
        dispenser->length = ui->dispenser_length->text().toInt()*1000/de_spacing_um;
        dispenser->width = ui->dispenser_width->text().toInt()*1000/de_spacing_um;
        dispenser->color = dispenser_color;
        ui->view->scene()->addItem(dispenser);
        allunits.prepend(dispenser);
        position += ui->merge_length->text().toInt() + 3;
        connect(dispenser, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}

//MERGE
void MainWindow::on_merge_create_clicked()
{
//    QBrush blueBrush(Qt::blue);
    blackpen.setWidth(6);
    int number = ui->merge_num->value();
    int position = 5;
    while(number--){
        unit *merge = new unit();
        merge->type = "merge";
        merge->xi = position;
        merge->yi = 10;
        merge->length = ui->merge_length->text().toInt();
        merge->width = ui->merge_width->text().toInt();
        merge->color = merge_color;
        ui->view->scene()->addItem(merge);
        allunits.prepend(merge);
        position += ui->merge_length->text().toInt() + 3;
        connect(merge, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}

//CYCLING
void MainWindow::on_cycling_create_clicked()
{
    int number = ui->cycling_num->value();
    int position = 5;
    while(number--){
        unit *cycle = new unit();
        cycle->type = "cycle";
        cycle->xi = position;
        cycle->yi = 15;
        cycle->de_type = 2;
        cycle->de_xnum = ui->cycling_length->text().toInt();
        cycle->de_ynum = ui->cycling_width->text().toInt();
        cycle->length = cycle->de_xnum*de2_length_mm*1000/de_spacing_um + cycle->de_xnum-1;
        cycle->width = cycle->de_ynum*de2_length_mm*1000/de_spacing_um + cycle->de_ynum-1;
        cycle->color = cycling_color;
        ui->view->scene()->addItem(cycle);
        allunits.prepend(cycle);
        position += ui->merge_length->text().toInt() + 3;
        connect(cycle, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}

//HEATER
void MainWindow::on_heater_create_clicked()
{
    int number = ui->heater_num->text().toInt();
    int position = 5;
    while(number--){
        unit *heat = new unit();
        heat->type = "heat";
        heat->xi = position;
        heat->yi = 20;      
        heat->de_xnum = 1;
        heat->de_ynum = 2;
        heat->color = heat_color;
        ui->view->scene()->addItem(heat);
        allunits.prepend(heat);
        position += 3;
        connect(heat, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}


void MainWindow::on_connect_btn_clicked()
{
    int count = 0;
    for(unit* unit : allunits){
        count += unit->de_xnum*unit->de_ynum;
    }
    int top = count / 2;
    int bot = count / 2 + count % 2;
    int start = chip_length_cm*10 - top*cp_length_mm;
    QBrush blackgr(Qt::black);


    for(int i = 0, x = start*pix_per_brick; i < top; i++){
        ui->view->scene()->addRect(x, 0.3*pix_per_brick, pix_per_brick*cp_length_mm*1000/de_spacing_um,
                                   pix_per_brick*cp_width_mm*1000/de_spacing_um, graypen, blackgr);
        x += pix_per_brick*(cp_length_mm*1000 + cp_spacing_um)/de_spacing_um;
    }
    for(int i = 0, x = start*pix_per_brick; i < bot; i++){
        ui->view->scene()->addRect(x, (chip_width_cm*10-cp_width_mm)*1000/de_spacing_um*pix_per_brick, pix_per_brick*cp_length_mm*1000/de_spacing_um,
                                   pix_per_brick*cp_width_mm*1000/de_spacing_um, graypen, blackgr);
        x += pix_per_brick*(cp_length_mm*1000 + cp_spacing_um)/de_spacing_um;
    }

    ui->view->scene()->update();
}

void MainWindow::debugging()
{
    qDebug() << "connected";
}

/////////////////////////////////////  SETTING  /////////////////////////////////////
void MainWindow::on_setting_btn_clicked()
{
    chip_setting *chip = new chip_setting(this);
    chip->setWindowTitle("Setting");
    connect(chip, SIGNAL(reset(chip_setting *)), this, SLOT(reset_setting(chip_setting *)));
    chip->show();
}

void MainWindow::reset_setting(chip_setting *new_chip)
{
    chip_length_cm = new_chip->chip_length_cm;
    chip_width_cm = new_chip->chip_width_cm;
    chip_border_mm = new_chip->chip_border_mm;

    cp_length_mm = new_chip->cp_length_mm;
    cp_width_mm = new_chip->cp_width_mm;

    de1_length_mm = new_chip->de1_length_mm;
    de1_width_mm = new_chip->de1_width_mm;
    de2_length_mm = new_chip->de2_length_mm;
    de2_width_mm = new_chip->de2_width_mm;

    de_spacing_um = new_chip->de_spacing_um;
    cp_spacing_um = new_chip->cp_spacing_um;
    line_width_mm = new_chip->line_width_mm;

    QGraphicsScene *newscene = CreateNewScene();

    delete(ui->view->scene());
    allunits.clear();
    this->mainscene = newscene;
    ui->view->setScene(newscene);
}


