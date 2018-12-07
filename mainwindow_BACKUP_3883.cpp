#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsSvgItem>
#define floatqDebug() qDebug() << fixed << qSetRealNumberPrecision(2)
int screen_width;
int screen_length;

/////////////////////////////////////  CHIP INFO  /////////////////////////////////////
bool deletemode = false;
bool linemode = false;

//Chip setting
int chip_length_cm = 6;
int chip_width_cm = 4;
int chip_border_mm = 3;

int cp_length_mm = 2;
int cp_width_mm = 4;

int de1_length_mm = 1;
int de1_width_mm = 5;
int de2_length_mm = 3;
int de2_width_mm = 3;

int de_spacing_um = 500;
int cp_spacing_um = 300;
int line_width_um = 200;
int line_width_pix = 10;
//Chip Parameters
int border_px;
int brick_xnum;
int brick_ynum;
int chip_width_px;
int chip_height_px;
int brick_x_start;
int brick_y_start;
int pix_per_brick = 20;         //How many PIXELs on the screen should we show to represent de_spacing
int cm_to_px = 10;              //how many PIXELS are in 1 cm

//Unit color
QColor merge_color      = QColor(255, 208, 166, 127);
QColor cycling_color    = QColor(1, 96, 177, 127);
QColor moving_color     = QColor(255, 184, 184, 127);
QColor dispenser_color  = QColor(215, 230, 144, 127);
QColor heat_color       = QColor(108, 137, 147, 127);

//Pen setting
QPen graypen(Qt::gray);
QPen redpen(Qt::red);
QPen blackpen(Qt::black);
QPen whitepen(Qt::white);
QPen linepen(Qt::gray);
QPen outlinepen;

//Brush setting
QBrush nullitem(QColor(94, 94, 94, 54));
QBrush blackbrush(Qt::black);

//Number of units
int num_merge = 0;
int num_dispenser = 0;
int num_move = 0;
int num_cycling = 0;
int num_heater = 0;
int num_de = 0;

//unit map
int unitmap[200][200];


float px_to_cm = 9921/21;
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

void MainWindow::GetScreenSize(int width, int length){
    screen_width = width;
    screen_length = length;
}

void MainWindow::ChipParameters(){
    pix_per_brick = fmin(600/((10*chip_length_cm*1000/de_spacing_um)),                  //calculate the size of each brick that fits the screeen the most
                         400/((10*chip_width_cm*1000/de_spacing_um)));
    cm_to_px = 10000/de_spacing_um*pix_per_brick;

    border_px = chip_border_mm*1000/de_spacing_um*pix_per_brick;                        //border width in pixel
    brick_xnum = (10*chip_length_cm - 2*chip_border_mm)*1000/de_spacing_um;             //how many background dots are needed in x-azis
    brick_ynum = (10*chip_width_cm - 2*chip_border_mm)*1000/de_spacing_um;              //how many background dots are needed in y-axis
    chip_height_px = brick_ynum*pix_per_brick+border_px*2;                              //chip_length in pixel
    chip_width_px = brick_xnum*pix_per_brick+border_px*2;                               //chip_width in pixel

    brick_x_start = (600 - chip_width_px)/2 + border_px;                                //where to start drawing dots in y-axis
    brick_y_start = (400 - chip_height_px)/2 + border_px;

    line_width_pix = pix_per_brick*line_width_um/de_spacing_um;
}

void MainWindow::OuterBorder(QGraphicsScene *scene){
    //1-1. outer border(nothing to do with real chip size, just the canvas size)
    scene->addLine(0, 0, scene->width(), 0, graypen);
    scene->addLine(0, scene->height(), scene->width(), scene->height(), graypen);
    scene->addLine(0, 0, 0, scene->height(), graypen);
    scene->addLine(scene->width(), 0, scene->width(), scene->height(), graypen);
}

void MainWindow::BackgroundGrid(QGraphicsScene *scene){
    //background grid (in dotted form)
    for(int i = 0; i <= brick_xnum; i++){
        for(int j = 0; j <= brick_ynum; j++){
            scene->addEllipse(brick_x_start+i*pix_per_brick, brick_y_start+j*pix_per_brick, 1, 1, graypen);
        }
    }
}

void MainWindow::ChipScaleDots(QGraphicsScene *scene){
    //chip scale                                                                        //show dots of scale near the chip border, each interval stands for 1cm in real size
    for(int i = 1; i < chip_length_cm; i++){
        scene->addEllipse(brick_x_start-border_px + i*cm_to_px, brick_y_start-border_px+chip_height_px-2, 1, 1, redpen);
    }
    for(int i = 1; i < chip_width_cm; i++){
        scene->addEllipse(brick_x_start-border_px+2, brick_y_start-border_px + i*cm_to_px, 1, 1, redpen);
    }
}

void MainWindow::ChipBorder(QGraphicsScene *scene){
    //chip border
    scene->addRect(brick_x_start-border_px, brick_y_start-border_px, chip_width_px, chip_height_px, redpen);    //the exact border of the chip
}

void MainWindow::ChipScale(QGraphicsScene *scene){
    //scale item                                                                     //show a 1cm scale
    scene->addRect(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px , 460, cm_to_px, 2, redpen);
    QGraphicsTextItem *text = scene->addText("1cm");
    text->setPos(brick_x_start - border_px + (chip_length_cm-1) * cm_to_px + (cm_to_px/2) - 12, 463);
}

void MainWindow::Info(){
    //Information
    ui->chip_length_label->setText(QString::number(chip_length_cm));
    ui->chip_width_label->setText(QString::number(chip_width_cm));
    ui->chip_border_label->setText(QString::number(chip_border_mm));
    ui->line_width_label->setText(QString::number(line_width_um));
    ui->cp_length_label->setText(QString::number(cp_length_mm));
    ui->cp_width_label->setText(QString::number(cp_width_mm));
    ui->cp_sapcing_label->setText(QString::number(cp_spacing_um));
    ui->de1_length_label->setText(QString::number(de1_length_mm));
    ui->de1_width_label->setText(QString::number(de1_width_mm));
    ui->de2_length_label->setText(QString::number(de2_length_mm));
    ui->de2_width_label->setText(QString::number(de2_width_mm));
    ui->de_spacing->setText(QString::number(de_spacing_um));

    ui->num_merge->setText(QString::number(num_merge));
    ui->num_dispenser->setText(QString::number(num_dispenser));
    ui->num_move->setText(QString::number(num_move));
    ui->num_cycling->setText(QString::number(num_cycling));
    ui->num_heater->setText(QString::number(num_heater));
}

void MainWindow::EmptyMessage(){
    ui->message->setStyleSheet(" font: 15pt American Typewriter; color: rgb(187, 210, 211);");
    ui->message->setText("");
}

void MainWindow::WarningMessage(QString message){
    ui->message->setStyleSheet("background-color : red; color : white;  font: 15pt American Typewriter; margin-left : 3px;");
    ui->message->setText(message);
}

//Create New Scene
QGraphicsScene* MainWindow::CreateNewScene(){
    QGraphicsScene * scene = new QGraphicsScene(0, 0, 600, 400, ui->view);
    scene->setBackgroundBrush(Qt::white);

    //Draw chip basics
    ChipParameters();
    OuterBorder(scene);
    BackgroundGrid(scene);
    ChipScaleDots(scene);
    ChipBorder(scene);
    ChipScale(scene);

    //Update Info Board
    Info();
    return scene;
}

graphicsscene* MainWindow::CreateLineScene(){
    graphicsscene* scene = new graphicsscene(ui->view);                                 //create a new scene
    scene->setSceneRect(QRectF(0, 0, 600, 400));
    scene->setBackgroundBrush(Qt::white);

    //Draw chip basics
    ChipParameters();
    OuterBorder(scene);
    BackgroundGrid(scene);
    ChipScaleDots(scene);
    ChipBorder(scene);
    ChipScale(scene);

    return scene;
}

void MainWindow::EnableCreateUnit(bool enable){
    ui->merge_create->setEnabled(enable);
    ui->dispenser_create->setEnabled(enable);
    ui->move_create->setEnabled(enable);
    ui->cycling_create->setEnabled(enable);
    ui->heater_create->setEnabled(enable);
}

//MODE LABEL                                                                            //shows the mode its in
void MainWindow::mode_label(bool bChecked){
    /////// LINE MODE ///////
    if(bChecked){
        deletemode = false;
        ui->eraser->setStyleSheet("background-color: rgb(42, 48, 58);");
        ui->view->setCursor(Qt::ArrowCursor);
        for(unit *item : allunits){
            if(item->type == "move"){                   //Show detail components for "move"
                if(item->tilt == 90){
                    for(int i = 0; i < item->de_ynum; i++){
                        if(item->de_type == 1){
                            DestroyRect << linescene->addRect(item->xi*pix_per_brick,
                                       (item->yi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                       pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                       pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                       redpen, nullitem);
                        } else {
                            DestroyRect << linescene->addRect(item->xi*pix_per_brick,
                                       (item->yi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                       pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                       pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                       redpen, nullitem);
                        }
                    }
                }
                else{
                    for(int i = 0; i < item->de_xnum; i++){
                        if(item->de_type == 1){
                            DestroyRect << linescene->addRect((item->xi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                       item->yi*pix_per_brick,
                                       pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                       pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                       redpen, nullitem);
                        } else {
                            DestroyRect << linescene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                       item->yi*pix_per_brick,
                                       pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                       pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                       redpen, nullitem);
                        }
                    }
                }
            }
            else if(item->type == "cycle"){             //Show detail components for 'cycling"
                for(int i = 0; i < item->de_xnum; i++){
                    for(int j = 0; j < item->de_ynum; j++){
                        if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1) continue;
                            DestroyRect << linescene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      (item->yi+j*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                      pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                      redpen, nullitem);
                    }
                }
            }
            else
                DestroyRect << linescene->addRect(item->xi*pix_per_brick, item->yi*pix_per_brick, item->length*pix_per_brick, item->width*pix_per_brick, redpen, nullitem);
        }
        ui->view->setScene(linescene);
        linemode = true;
        ui->mode_label->setText("LINE MODE");
        QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/cursor.png");
        QCursor pencil = QCursor(*e, -10, -10);
        ui->view->setCursor(pencil);
    }
    /////// UNIT MODE ///////
    else{
        EmptyMessage();
        deletemode = false;
        ui->eraser->setStyleSheet("background-color: rgb(42, 48, 58);");
        ui->view->setCursor(Qt::ArrowCursor);

        for(QGraphicsItem* rect: DestroyRect){
             delete rect;
        }
        for(QGraphicsLineItem* line: DestroyLine){
            delete line;
        }
        DestroyRect.clear();        
        DestroyLine.clear();
        linepen.setColor(Qt::gray);
        linepen.setWidth(line_width_pix);
        for(line *head : linescene->alllines){
            line *current_seg = head;
            while(current_seg->next != NULL){
                DestroyLine << mainscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
                current_seg = current_seg->next;
            }
            DestroyLine << mainscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
        }
        ui->view->setScene(mainscene);
        linemode = false;
        ui->mode_label->setText("UNIT MODE");
        ui->view->setCursor(Qt::ArrowCursor);
    }
}

////////////////////////////////////////  ACTIONS  /////////////////////////////////////
void MainWindow::new_chip_clicked()
{
    this->save_svg();
    QGraphicsScene *newscene = CreateNewScene();
    delete(ui->view->scene());
    allunits.clear();
    this->mainscene = newscene;
    ui->view->setScene(newscene);
}

//SAVE CHIP
void MainWindow::save_svg()
{
     QString newPath = QFileDialog::getSaveFileName(this, "Save Chip .txt", path, tr("TXT files (*.txt)"));
     if(newPath.isEmpty()) return;

      QFile file(newPath);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
          return;

      QTextStream out(&file);

      out << chip_length_cm << "\n" << chip_width_cm << "\n" << chip_border_mm << "\n";
      out << cp_length_mm << "\n" << cp_width_mm << "\n";
      out << de1_length_mm << "\n" << de1_width_mm << "\n" << de2_length_mm  << "\n"<< de2_width_mm << "\n";

      out << de_spacing_um << "\n";
      out << cp_spacing_um << "\n";
      out << line_width_um  << "\n";

      out << num_merge << "\n" << num_dispenser << "\n" << num_move << "\n" << num_cycling << "\n" <<num_heater << "\n" << num_de << "\n";
      out << allunits.size() << "\n";
      for(unit *item: allunits){
        out << item->type << "\n";
        out << item->xi << "\n" << item->yi  << "\n";
        if(item->type == "move" || item->type == "cycle"){
            out << item->de_type << "\n" << item->de_xnum << "\n" << item->de_ynum << "\n";
        }
        if(item->type == "move"){
            out << item->tilt << "\n";
        }
        out << item->length << "\n" << item->width << "\n";
        out << item->color.red() << "\n" << item->color.green() << "\n" << item->color.blue() << "\n" << item->color.alpha() << "\n";
      }

      out << linescene->alllines.size() << "\n";

      line *current_seg;
      for(line *head : linescene->alllines){
          current_seg = head;
          while(current_seg->next != nullptr){
              out << current_seg->x[0] << "\n" << current_seg->y[0] << "\n";
              current_seg = current_seg->next;
          }
          out << current_seg->x[0] << "\n" << current_seg->y[0] << "\n";
          out << current_seg->x[1] << "\n" << current_seg->y[1] << "\n";
          out << 1000 << "\n";  //END LINE
      }

//      for(line *turnline : linescene->alllines){
//          out << turnline->segments << "\n";
//          for(int i=0; i<=turnline->segments; i++){
//              out << turnline->x[i] << "\n" << turnline->y[i] << "\n";
//          }
//      }
}

void MainWindow::load_svg_clicked()
{
    DestroyRect.clear();
    allunits.clear();
    QString newPath = QFileDialog::getOpenFileName(this, "Open Chip", path, tr("TXT files (*.txt)"));
    if(newPath.isEmpty())return;

    QFile file(newPath);
       if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
           return;

    QTextStream in(&file);

    //Import Chip
    chip_length_cm = in.readLine().toInt();
    chip_width_cm = in.readLine().toInt();
    chip_border_mm = in.readLine().toInt();
    cp_length_mm = in.readLine().toInt();
    cp_width_mm = in.readLine().toInt();

    de1_length_mm = in.readLine().toInt();
    de1_width_mm = in.readLine().toInt();
    de2_length_mm  = in.readLine().toInt();
    de2_width_mm = in.readLine().toInt();

    de_spacing_um = in.readLine().toInt();
    cp_spacing_um = in.readLine().toInt();
    line_width_um  = in.readLine().toInt();
    //line_width_pix = in.readLine().toInt();

    num_merge = in.readLine().toInt();
    num_dispenser = in.readLine().toInt();
    num_move = in.readLine().toInt();
    num_cycling = in.readLine().toInt();
    num_heater = in.readLine().toInt();
    num_de = in.readLine().toInt();

    //Set Chip to mainscene
    mainscene = CreateNewScene();
    linescene = CreateLineScene();

    //Import Units
    int num_of_units = in.readLine().toInt();
    qDebug() << "num of units" << num_of_units;
    while(num_of_units){
        unit *item = new unit();
        item->type = in.readLine();
        item->xi = in.readLine().toInt();
        item->yi = in.readLine().toInt();
        if(item->type == "move" || item->type == "cycle"){
            item->de_type = in.readLine().toInt();
            item->de_xnum = in.readLine().toInt();
            item->de_ynum = in.readLine().toInt();
        }
        if(item->type == "move"){
            item->tilt = in.readLine(). toInt();
        }
        item->length = in.readLine().toInt();
        item->width = in.readLine().toInt();
        int R, G, B, Alpha;
        R = in.readLine().toInt();
        G = in.readLine().toInt();
        B = in.readLine().toInt();
        Alpha = in.readLine().toInt();
        item->color = QColor(R, G, B, Alpha);
        mainscene->addItem(item);
        allunits.prepend(item);
        connect(item, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
        num_of_units--;
    }
    //Import Lines
    int num_of_lines = in.readLine().toInt();
    qDebug() << "num of lines" << num_of_lines;
    while(num_of_lines){
        line *head = new line();
        line *current_seg = new line();

        bool ishead = true;
        float x, y;
        while(1){
            x = in.readLine().toFloat();
            if(x == 1000.00){
                break;
            }
            y = in.readLine().toFloat();
            floatqDebug() << x << y;
            line *new_seg = new line();
            new_seg->x[0] = x;
            new_seg->y[0] = y;
            if(ishead){
                new_seg->previous = NULL;
                new_seg->next = NULL;
                head = new_seg;
                current_seg = new_seg;
                ishead = false;
            }
            else{
                current_seg->x[1] = x;
                current_seg->y[1] = y;
                current_seg->next = new_seg;
                new_seg->previous = current_seg;
                current_seg = new_seg;
            }
        }
        floatqDebug() << x;
        current_seg = current_seg->previous;
        current_seg->next = NULL;

        linescene->AddTurnline(head);
    num_of_lines--;
    }

    linepen.setColor(Qt::gray);
    linepen.setWidth(line_width_pix);

    for(line *head : linescene->alllines){
        line *current_seg = head;
        while(current_seg->next != NULL){
            DestroyLine << mainscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
            current_seg = current_seg->next;
        }
        DestroyLine << mainscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
    }



//    while(num_of_lines){
//        line *turnline = new line();
//        turnline->segments = in.readLine().toInt();
//        qDebug() << "SEGMENTS" << turnline->segments;
//        for(int i=0; i<=turnline->segments; i++){
//            turnline->x[i] = in.readLine().toFloat();
//            turnline->y[i] = in.readLine().toFloat();
//        }
//        linescene->AddTurnline(turnline);
//        num_of_lines--;
//    }
//    linepen.setColor(Qt::gray);
//    linepen.setWidth(line_width_pix);


//    for(line *turnline : linescene->alllines){
//        for(int i=0; i<turnline->segments; i++){
//            DestroyLine << mainscene->addLine(turnline->x[i], turnline->y[i], turnline->x[i+1], turnline->y[i+1], linepen);
//        }
//    }
    ui->view->setScene(mainscene);
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


       // border in REAL SIZE
       export_scene->addRect(0, 0, chip_length_cm*px_to_cm, chip_width_cm*px_to_cm, graypen);

       //units in REAL SIZE
       for(unit *item : allunits){
            unit *export_item = new unit();
            //export_item = item;
            export_item->xi = (item->xi - (brick_x_start-border_px)/pix_per_brick) * de_spacing_um* px_to_cm  / 10000 / pix_per_brick;
            export_item->yi = item->yi * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->length = item->length * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->width = item->width *de_spacing_um * px_to_cm  / 10000 / pix_per_brick;

            export_item->color = QColor(Qt::black);
            if(item->type == "move"){
                for(int i = 0; i < item->de_xnum; i++){
                    if(item->de_type == 1){
                        export_scene->addRect(((item->xi+i*(de1_length_mm*1000/de_spacing_um + 1)) - (brick_x_start-border_px)/pix_per_brick )* de_spacing_um* px_to_cm  / 10000,
                                   item->yi* de_spacing_um * px_to_cm  / 10000,
                                   px_to_cm/10*de1_length_mm,
                                   px_to_cm/10*de1_width_mm,
                                   blackpen, blackbrush);
                    } else {
                        export_scene->addRect(((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1)) - (brick_x_start-border_px)/pix_per_brick )* de_spacing_um* px_to_cm  / 10000,
                                   item->yi* de_spacing_um * px_to_cm  / 10000,
                                   px_to_cm/10*de2_length_mm,
                                   px_to_cm/10*de2_width_mm,
                                   blackpen, blackbrush);
                    }
                }
            }
            else if(item->type == "cycle"){
                for(int i = 0; i < item->de_xnum; i++){
                    for(int j = 0; j < item->de_ynum; j++){
                        if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1) continue;
                            export_scene->addRect(((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))- (brick_x_start-border_px)/pix_per_brick)* de_spacing_um* px_to_cm  / 10000,
                                      (item->yi+j*(de2_length_mm*1000/de_spacing_um + 1))* de_spacing_um * px_to_cm  / 10000,
                                      px_to_cm / 10*de2_length_mm,
                                      px_to_cm / 10*de2_length_mm,
                                      blackpen, blackbrush);
                    }
                }
            }
            else{
                export_scene->addItem(export_item);
            }


       }
       linepen.setColor(Qt::black);
       linepen.setWidth(line_width_um * px_to_cm  / 10000);
       for(line *head : linescene->alllines){
           line *current_seg = head;
           while(current_seg->next != NULL){
               export_scene->addLine(current_seg->x[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->x[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
               current_seg = current_seg->next;
           }
           export_scene->addLine(current_seg->x[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->x[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
       }
//       for(line *turnline : linescene->alllines){
//           for(int i=0; i<turnline->segments; i++){
//               export_scene->addLine(turnline->x[i]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->y[i]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->x[i+1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->y[i+1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
//           }
//       }
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

       // border in REAL SIZE
       export_scene->addRect(0, 0, chip_length_cm*px_to_cm, chip_width_cm*px_to_cm, graypen);

       //units in REAL SIZE
       for(unit *item : allunits){
            unit *export_item = new unit();
            //export_item = item;
            export_item->xi = (item->xi - (brick_x_start-border_px)/pix_per_brick) * de_spacing_um* px_to_cm  / 10000 / pix_per_brick;
            export_item->yi = item->yi * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->length = item->length * de_spacing_um * px_to_cm  / 10000 / pix_per_brick;
            export_item->width = item->width *de_spacing_um * px_to_cm  / 10000 / pix_per_brick;

            export_item->color = QColor(Qt::black);
            if(item->type == "move"){
                for(int i = 0; i < item->de_xnum; i++){
                    if(item->de_type == 1){
                        export_scene->addRect(((item->xi+i*(de1_length_mm*1000/de_spacing_um + 1)) - (brick_x_start-border_px)/pix_per_brick )* de_spacing_um* px_to_cm  / 10000,
                                   item->yi* de_spacing_um * px_to_cm  / 10000,
                                   px_to_cm/10*de1_length_mm,
                                   px_to_cm/10*de1_width_mm,
                                   blackpen, blackbrush);
                    } else {
                        export_scene->addRect(((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1)) - (brick_x_start-border_px)/pix_per_brick )* de_spacing_um* px_to_cm  / 10000,
                                   item->yi* de_spacing_um * px_to_cm  / 10000,
                                   px_to_cm/10*de2_length_mm,
                                   px_to_cm/10*de2_width_mm,
                                   blackpen, blackbrush);
                    }
                }
            }
            else if(item->type == "cycle"){
                for(int i = 0; i < item->de_xnum; i++){
                    for(int j = 0; j < item->de_ynum; j++){
                        if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1) continue;
                                export_scene->addRect(((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))- (brick_x_start-border_px)/pix_per_brick)* de_spacing_um* px_to_cm  / 10000,
                                          (item->yi+j*(de2_length_mm*1000/de_spacing_um + 1))* de_spacing_um * px_to_cm  / 10000,
                                          px_to_cm / 10*de2_length_mm,
                                          px_to_cm / 10*de2_length_mm,
                                          blackpen, blackbrush);
                    }
                }
            }
            else{
                export_scene->addItem(export_item);
            }
       }
       linepen.setColor(Qt::black);
       linepen.setWidth(line_width_um * px_to_cm  / 10000);
       for(line *head : linescene->alllines){
           line *current_seg = head;
           while(current_seg->next != NULL){
               export_scene->addLine(current_seg->x[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->x[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
               current_seg = current_seg->next;
           }
           export_scene->addLine(current_seg->x[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[0]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->x[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, current_seg->y[1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
       }
//       for(line *turnline : linescene->alllines){
//           for(int i=0; i<turnline->segments; i++){
//               export_scene->addLine(turnline->x[i]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->y[i]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->x[i+1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, turnline->y[i+1]* de_spacing_um * px_to_cm  / 10000 / pix_per_brick, linepen);
//           }
//       }
       export_scene->render(&p);
       p.end();
}

// CLOSE
void MainWindow::close_clicked()
{
    save_yn_dialog *want = new save_yn_dialog(this);
    want->setWindowTitle("Do you want to save this chip?");
    connect(want, SIGNAL(yes_save(save_yn_dialog *)), this, SLOT(close_window(save_yn_dialog *)));
    want->show();
}

void MainWindow::close_window(save_yn_dialog *s)
{
    if(s->want_to_save){
        this->save_svg();
        this->close();
    }
    else{
        this->close();
        qDebug() << "no";
    }

}

/////////////////////////////////////  TOOL  BAR  /////////////////////////////////////
//DELETE (ERASER)
void MainWindow::on_eraser_clicked()
{
    deletemode = 1-deletemode;
    qDebug() << "number of units : " << allunits.count();
    qDebug() << "number of lines : "<< linescene->alllines.count();
    qDebug() << "DELETE MODE : " <<deletemode;

    if(deletemode == true){
        QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/eraser_cursor.png");
        QCursor cursorErase = QCursor(*e, -10, -4);
        ui->eraser->setStyleSheet("background-color: rgb(64, 72, 91);");
        ui->view->setCursor(cursorErase);
    }
    else {
        if(linemode){
            ui->eraser->setStyleSheet("background-color: rgb(42, 48, 58);");
            QPixmap *e = new QPixmap(":/MainWindow/Icons/Icons/cursor.png");
            QCursor pencil = QCursor(*e, -10, -10);
            ui->view->setCursor(pencil);
        }
        else{
            ui->eraser->setStyleSheet("background-color: rgb(42, 48, 58);");
            ui->view->setCursor(Qt::ArrowCursor);
        }
    }
}

//remove the deleted item from allunits list
void MainWindow::delete_from_list(unit *item)
{
    if(item->type == "merge"){ num_merge--; num_de -= 1;}
    if(item->type == "dispenser"){ num_dispenser--; num_de -= 1;}
    if(item->type == "move"){ num_move--; num_de -= item->de_xnum*item->de_ynum;}
    if(item->type == "cycle"){ num_cycling--; num_de -= (item->de_xnum + item->de_ynum - 2) * 2;}
    if(item->type == "heater"){ num_heater--; num_de -= 2;}
    Info();
    allunits.removeOne(item);
    delete item;
}

void MainWindow::on_zoomin_clicked()
{
    zoom_factor *= 1.1;
    ui->view->setTransform(QTransform().scale(zoom_factor, zoom_factor));
}

void MainWindow::on_zoomout_clicked()
{
    zoom_factor /= 1.1;
    ui->view->setTransform(QTransform().scale(zoom_factor, zoom_factor));
}

//AUTO CONNECT
void MainWindow::on_connect_btn_clicked()
{

    int xsize = brick_xnum;//(chip_length_cm*10 - 2*chip_border_mm)*1000 / de_spacing_um;
    int ysize = brick_ynum;//(chip_width_cm*10 - 2*chip_border_mm)*1000 / de_spacing_um;
    float shift = chip_border_mm*1000 / de_spacing_um;

    struct edge{
        int flow, cap, cost;
        unsigned to, rev;
        edge(unsigned to, int cap, int cost, unsigned rev){
            this->to = to;
            this->cap = cap;
            this->cost = cost;
            this->rev = rev;
            this->flow = 0;
        }
    };

    std::vector<std::vector<edge>> graph(2*unsigned(xsize*ysize)+2);
    auto addEdge = [&graph](unsigned s, unsigned t, int cost, int cap){
        graph[s].push_back(edge(t, cap, cost, graph[t].size()));
        graph[t].push_back(edge(s, 0, -cost, graph[s].size()-1));
    };

    // unitmap setup
    memset(unitmap, 0, sizeof(unitmap));
    int unit_id = 2;
    int cpad_id = -2;
    bool source[200][200] = {{false}};

    for(unit *item : allunits){
        if(item->type == "move"){
            int de_length_mm = (item->de_type == 1) ? de1_length_mm : de2_length_mm;
            int de_width_mm = (item->de_type == 1) ? de1_width_mm : de2_width_mm;
            int unit_length = ((item->tilt == 0) ? de_length_mm : de_width_mm)*1000/de_spacing_um;
            int unit_width = ((item->tilt == 0) ? de_width_mm : de_length_mm)*1000/de_spacing_um;
            int i = 0, j = 0;
            for( ; i < item->de_xnum && j < item->de_ynum; ){
                for(int k = 0; k <= unit_length; k++)
                    for(int l = 0; l <= unit_width; l++)
                        unitmap[int(item->xi - shift)+i*(unit_length+1) + k][int(item->yi - shift)+j*(unit_width+1) + l] = unit_id;//(j == int(unit_length/2) || k == int(unit_width/2)) ? 1 : unit_id;
                source[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift)+j*(unit_width+1) + unit_width/2] = true;
                unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift)+j*(unit_width+1) + unit_width*1] = 1;
                unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift)+j*(unit_width+1) + unit_width*0] = 1;
                unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length*1][int(item->yi - shift)+j*(unit_width+1) + unit_width/2] = 1;
                unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length*0][int(item->yi - shift)+j*(unit_width+1) + unit_width/2] = 1;
                unit_id ++;
                if(item->tilt == 0){
                    i++;
                } else {
                    j++;
                };
            }

        } else if(item->type == "cycle"){
            int unit_length = de2_length_mm*1000/de_spacing_um;
            for(int i = 0; i < item->de_xnum; i++){
                for(int j = 0; j < item->de_ynum; j++){
                    if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1)
                        continue;
                    qDebug() << i << j;
                    for(int k = 0; k <= unit_length; k++)
                        for (int l = 0; l <= unit_length; l++)
                            unitmap[int(item->xi - shift)+i*(unit_length+1) + k][int(item->yi - shift)+j*(unit_length+1) + l] = unit_id;//(k == int(unit_length/2) || l == int(unit_length/2)) ? 1 : unit_id;
                    source[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift) +j*(unit_length+1)+ unit_length/2] = true;
                    unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift) +j*(unit_length+1)+ unit_length] = 1;
                    unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length/2][int(item->yi - shift) +j*(unit_length+1)+ 0] = 1;
                    unitmap[int(item->xi - shift)+i*(unit_length+1) + unit_length][int(item->yi - shift) +j*(unit_length+1)+ unit_length/2] = 1;
                    unitmap[int(item->xi - shift)+i*(unit_length+1) + 0][int(item->yi - shift) +j*(unit_length+1)+ unit_length/2] = 1;
                    unit_id ++;
                }
            }

        } else if(item->type == "dispenser"){
            for(int i = 0; i <= item->length; i++)
                for(int j = 0; j <= item->width; j++)
                    unitmap[int(item->xi - shift) + i][int(item->yi - shift) + j] = unit_id;//(i == int(item->length/2) || j == int(item->width/2)) ? 1 : unit_id;
            source[int(item->xi - shift) + int(item->length/2)][int(item->yi - shift) + int(item->width/2)] = true;
            unitmap[int(item->xi - shift) + int(item->length*1)][int(item->yi - shift) + int(item->width/2)] = 1;
            unitmap[int(item->xi - shift) + int(item->length*0)][int(item->yi - shift) + int(item->width/2)] = 1;
            unitmap[int(item->xi - shift) + int(item->length/2)][int(item->yi - shift) + int(item->width*1)] = 1;
            unitmap[int(item->xi - shift) + int(item->length/2)][int(item->yi - shift) + int(item->width*0)] = 1;
            unit_id ++;

        } else if(item->type == "controlpad"){
            for(int i = 0; i <= item->length; i++)
                for(int j = 0; j <= item->width; j++)
                    unitmap[int(item->xi - shift) + i][int(item->yi - shift) + j] = (i == int(item->length/2) /*|| j == int(item->width/2)*/) ? -1 : cpad_id;
            cpad_id --;
        }
    }

    qDebug() << "total de is " << num_de;

    for(int i = 0; i < xsize; i++){
        QString debug;
        for(int j = 0; j < ysize; j++){
            debug += QString::number(unitmap[i][j]);
        }
        qDebug() << debug;
    }


    // set source and target
    unsigned s = unsigned(xsize*ysize*2);
    unsigned t = unsigned(xsize*ysize*2+1);

    // flow node connection
    for(int i = 0; i < xsize; i++){
        for(int j = 0; j < ysize; j++){
            unsigned from = unsigned(i*ysize+j);
            unsigned to_d = unsigned((i+1)*ysize+j); //down (actually right on chip
            unsigned to_r = unsigned(i*ysize+j+1); //right (actually down on chip
            // self connection for node capacity
            addEdge(from+unsigned(xsize*ysize), from, 0, 1);

            // neighbors connection
            if (i != xsize-1) {
                if(unitmap[i][j] == unitmap[i+1][j] /*|| (unitmap[i][j] * unitmap[i+1][j] > 0)*/){
                    if(unitmap[i][j] == 1 && unitmap[i+1][j] == 1) continue;
                    addEdge(from, to_d+unsigned(xsize*ysize), (unitmap[i][j] == 0) ? 1 : 0, 1);
                    addEdge(to_d, from+unsigned(xsize*ysize), (unitmap[i][j] == 0) ? 1 : 0, 1);
                } else if(unitmap[i][j] > unitmap[i+1][j]){
                    if(unitmap[i][j] == 1 || unitmap[i+1][j] == -1 || unitmap[i+1][j] == 1)
                        addEdge(from, to_d+unsigned(xsize*ysize), 1, 1);
                    else
                        ;
                } else {
                    if(unitmap[i+1][j] == 1 || unitmap[i][j] == -1 || unitmap[i][j] == 1)
                        addEdge(to_d, from+unsigned(xsize*ysize), 1, 1);
                    else;
                }
            }
            if (j != ysize-1) {
                if(unitmap[i][j] == unitmap[i][j+1] /*|| (unitmap[i][j] * unitmap[i][j+1] > 0)*/){
                    if(unitmap[i][j] == 1 && unitmap[i][j+1] == 1) continue;
                    addEdge(from, to_r+unsigned(xsize*ysize), (unitmap[i][j] == 0) ? 1 : 0, 1);
                    addEdge(to_r, from+unsigned(xsize*ysize), (unitmap[i][j] == 0) ? 1 : 0, 1);
                } else if(unitmap[i][j] > unitmap[i][j+1]){
                    if(unitmap[i][j] == 1 || unitmap[i][j+1] == -1 || unitmap[i][j+1] == 1)
                        addEdge(from, to_r+unsigned(xsize*ysize), 1, 1);
                    else
                        ;
                } else {
                    if(unitmap[i][j+1] == 1 || unitmap[i][j] == -1 || unitmap[i][j] == 1)
                        addEdge(to_r, from+unsigned(xsize*ysize), 1, 1);
                    else
                        ;
                }
            }

            // s/t connection
            if(unitmap[i][j] > 1){
                //if(!(i < xsize-1 && (unitmap[i+1][j] == 1 || unitmap[i+1][j] == unitmap[i][j])) &&
                  // !(j < ysize-1 && (unitmap[i][j+1] == 1 || unitmap[i][j+1] == unitmap[i][j])))
                if(source[i][j] == true)
                    addEdge(s, from+unsigned(xsize*ysize), 1, 1);
            } else if(unitmap[i][j] <= -1){
                if(!(i < xsize-1 && (unitmap[i+1][j] == -1 || unitmap[i+1][j] == unitmap[i][j])) &&
                   !(j < ysize-1 && (unitmap[i][j+1] == -1 || unitmap[i][j+1] == unitmap[i][j])))
                    addEdge(from, t, 1, 1);
            }
        }
    }

    int count = 0;
    /*for(int i = 0; i < xsize; i++)
        for(int j = 0; j < ysize; j++)
            qDebug() << unitmap[i][j];*/
    for(unit* unit : allunits){
        count += unit->de_xnum*unit->de_ynum;
        qDebug() << unit->xi << unit->yi;
    }

    unsigned n = graph.size();
    std::vector<int> distance(n);
    std::vector<int> currflow(n);
    std::vector<int> prevedge(n);
    std::vector<int> prevnode(n);


    // find path by bellmanFord
    auto bellmanFord = [n, s, &graph, &distance, &currflow, &prevedge,&prevnode](){
        std::fill(distance.begin(), distance.end(), INT_MAX);
        distance[s] = 0;
        currflow[s] = INT_MAX;
        std::vector<bool> inqueue(n);
        std::queue<unsigned> q;
        q.push(s);
        while(!q.empty()){
            unsigned u = q.front();
            q.pop();
            inqueue[u] = false;
            for (unsigned i = 0; i < graph[u].size(); i++) {
                edge e = graph[u][i];
                if (e.flow >= e.cap)
                    continue;
                unsigned v = e.to;
                int ndist = distance[u] + e.cost;
                if (distance[v] > ndist) {
                    distance[v] = ndist;
                    prevnode[v] = int(u);
                    prevedge[v] = int(i);
                    currflow[v] = std::min(currflow[u], e.cap - e.flow);
                    if (!inqueue[v]) {
                        inqueue[v] = true;
                        q.push(v);
                    }
                }
            }
        }
    };

    // min cost flow
    int flow = 0;
    int flowCost = 0;
    int maxflow = num_de;

    while (flow < maxflow) {
        bellmanFord();
        if (distance[unsigned(t)] == INT_MAX)
            break;
        int df = std::min(currflow[t], maxflow - flow);
        flow += df;
        for (unsigned v = t; v != s; v = unsigned(prevnode[v])) {
            edge* e;
            e = &graph[unsigned(prevnode[v])][unsigned(prevedge[v])];
            e->flow += df;
            graph[v][e->rev].flow -= df;
            flowCost += df * e->cost;
            //delete(e);
        }
    }

    qDebug() << flow << flowCost;

    std::vector<line*>newlines;

    for(int i = 0; i < xsize; i++){
        for(int j = 0; j < ysize; j++){
            for(edge e : graph[unsigned(i*ysize+j)]){
                if(e.flow > 0 && e.to != s && e.to != t){
                    unsigned to_x = (e.to-unsigned(xsize*ysize)) / unsigned(ysize);
                    unsigned to_y = (e.to-unsigned(xsize*ysize)) % unsigned(ysize);
                    qDebug()<<i<<j<<to_x<<to_y;
                    if(unitmap[i][j] * unitmap[to_x][to_y] > 0)
                        continue;
                    qreal startx = (i+shift)*pix_per_brick;
                    qreal starty = (j+shift)*pix_per_brick;
                    qreal endx = pix_per_brick*(shift+((to_x < i) ? i-1 : (to_x > i) ? i+1 : i));
                    qreal endy = pix_per_brick*(shift+((to_y < j) ? j-1 : (to_y > j) ? j+1 : j));
                    //ui->view->scene()->addRect(startx, starty, lengthx, lengthy, QPen(Qt::black), QBrush(Qt::black));
                    qDebug()<<startx<<starty<<endx<<endy;
                    line *newline = new line();
                    newline->next = nullptr;
                    newline->previous = nullptr;
                    newline->x[0] = startx;
                    newline->y[0] = starty;
                    newline->x[1] = endx;
                    newline->y[1] = endy;
                    newlines.push_back(newline);
                }
            }
        }
    }
    qDebug()<<"end build line"<<newlines.size();
    for(auto newline : newlines){
        for(auto checkline : newlines){
            //qDebug() << newline->x[0] << checkline->x[1];
            //qDebug() << newline->x[1] << checkline->x[0];
            if(newline == checkline){
                continue;
            } else if(abs(newline->x[0] - checkline->x[1]) <= 0.3*pix_per_brick && abs(newline->y[0] - checkline->y[1]) <= 0.3*pix_per_brick){
                newline->previous = checkline;
                checkline->next = newline;
            } else if(abs(newline->x[1] - checkline->x[0]) <= 0.3*pix_per_brick && abs(newline->y[1] - checkline->y[0]) <= 0.3*pix_per_brick){
                newline->next = checkline;
                checkline->previous = newline;
            }
        }
    }
    qDebug()<<"end line connection";
    std::vector<line*>mergeline;
    for(auto line : newlines){
        if(line->previous == nullptr)
            mergeline.push_back(line);
    }
    qDebug()<<mergeline.size();
    for(auto line: linescene->alllines){
        linescene->delete_from_list(line);
    }

    for(auto line: mergeline){
        linescene->AddTurnline(line);
    }
    qDebug() << linescene->alllines.size();
    //linescene->update();
    ui->view->scene()->update();
}

//CREATE CONTROL PAD
void MainWindow::on_controlpad_btn_clicked()
{
    //int size = allunits.size();
    //int mm_to_px = 1000/de_spacing_um*pix_per_brick;
    int cp_xi_start = chip_border_mm*1000 / de_spacing_um;
    int cp_yi_start = chip_border_mm*1000 / de_spacing_um;

    for(int i=0; i<num_de; i++){
        //mainscene->addRect(cp_x_start, cp_y_start, cp_length_mm*mm_to_px, cp_width_mm*mm_to_px, graypen, QColor(94, 93, 93, 54));
        //cp_x_start += cp_length_mm*mm_to_px + cp_spacing_um * mm_to_px / 1000;
        unit *cp = new unit();
        cp->type = "controlpad";
        cp->xi = cp_xi_start;
        cp->yi = cp_yi_start;
        cp->length = cp_length_mm*1000/de_spacing_um;
        cp->width = cp_width_mm*1000/de_spacing_um;
        cp->color = Qt::black;
        ui->view->scene()->addItem(cp);
        allunits.prepend(cp);

        if(cp_xi_start+cp->length+1 >= brick_xnum){
            cp_xi_start = chip_border_mm*1000 / de_spacing_um;
            cp_yi_start += int(brick_ynum - cp->width);
        } else {
            cp_xi_start += cp->length+1;
        }
        //cp_xi_start = (cp_xi_start+cp->length+1 >= brick_xnum) ? chip_border_mm*1000 / de_spacing_um : cp_xi_start + cp->length+1;
        //cp_yi_start = ((i+1)*(cp->length+1) >= brick_xnum) ? int(brick_ynum - cp->width): cp_yi_start;
        qDebug() << brick_xnum;
        connect(cp, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
    }
}


/////////////////////////////////////  CREATE UNITS  /////////////////////////////////////
//MERGE
void MainWindow::on_merge_create_clicked()
{
    if(linemode){
        WarningMessage("Cannot add components in LINE mode!");
    }
    else if(ui->merge_length->text().isEmpty() && ui->merge_width->text().isEmpty())
    {
        WarningMessage("Missing Length and Width");
    }
    else if(ui->merge_length->text().isEmpty()){
        WarningMessage("Missing Length");
    }
    else if(ui->merge_width->text().isEmpty()){
        WarningMessage("Missing Width");
    }
    else{
        EmptyMessage();
        blackpen.setWidth(6);
        int number = ui->merge_num->value();
        int position = 5;
        num_merge += number;
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
            num_de += 1;
        }
        ui->num_merge->setText(QString::number(num_merge));
    }
}

//DISPENSER
void MainWindow::on_dispenser_create_clicked()
{
    if(linemode){
        WarningMessage("Cannot add components in LINE mode!");
    }
    else if(ui->dispenser_length->text().isEmpty() && ui->dispenser_width->text().isEmpty())
    {
        WarningMessage("Missing Length and Width");
    }
    else if(ui->dispenser_length->text().isEmpty()){
        WarningMessage("Missing Length");
    }
    else if(ui->dispenser_width->text().isEmpty()){
        WarningMessage("Missing Width");
    }
    else{
        EmptyMessage();
        int number = ui->dispenser_num->value();
        int position = 5;
        num_dispenser += number;
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
            num_de += 1;
        }
        ui->num_dispenser->setText(QString::number(num_dispenser));
    }
}

//MOVE
void MainWindow::on_move_create_clicked()
{
    if(linemode){
        WarningMessage("Cannot add components in LINE mode!");
    }
    else if(ui->move_size->text().isEmpty()){
        WarningMessage("Missing Size");
    }
    else{
        EmptyMessage();
        int number = ui->move_num->value();
        int position = 5;
        num_move += number;
        while(number--){
            unit *move = new unit();
            move->type = "move";
            move->xi = position;
            move->yi = 0;
            move->tilt = ui->move_tilt->text().toInt();
            move->de_type = ui->move_electrod->value();
            if(move->tilt == 90){
                move->de_xnum = 1;
                move->de_ynum = ui->move_size->text().toInt();
                if(move->de_type == 1){
                    move->length = move->de_xnum*de1_width_mm*1000/de_spacing_um;
                    move->width = move->de_ynum*de1_length_mm*1000/de_spacing_um + move->de_ynum-1;
                }
                else{
                    move->length = move->de_xnum*de2_width_mm*1000/de_spacing_um;
                    move->width = move->de_ynum*de2_length_mm*1000/de_spacing_um + move->de_ynum-1;
                }
            }
            else{
                move->de_xnum = ui->move_size->text().toInt();
                move->de_ynum = 1;
                if(move->de_type == 1){
                    move->length = move->de_xnum*de1_length_mm*1000/de_spacing_um + move->de_xnum-1;
                    move->width = move->de_ynum*de1_width_mm*1000/de_spacing_um;
                }
                else{
                    move->length = move->de_xnum*de2_length_mm*1000/de_spacing_um + move->de_xnum-1;
                    move->width = move->de_ynum*de2_width_mm*1000/de_spacing_um;
                }
            }
            move->color = moving_color;
            ui->view->scene()->addItem(move);
            allunits.prepend(move);
            position += 5;
            connect(move, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
            num_de += move->de_xnum * move->de_ynum;
        }
        ui->num_move->setText(QString::number(num_move));
    }
}

//CYCLING
void MainWindow::on_cycling_create_clicked()
{
    if(linemode){
        WarningMessage("Cannot add components in LINE mode!");
    }
    else if(ui->cycling_length->text().isEmpty() && ui->cycling_width->text().isEmpty())
    {
        WarningMessage("Missing Length and Width");
    }
    else if(ui->cycling_length->text().isEmpty()){
        WarningMessage("Missing Legth");
    }
    else if(ui->cycling_width->text().isEmpty()){
        WarningMessage("Missing Width");
    }
    else{
        EmptyMessage();
        int number = ui->cycling_num->value();
        int position = 5;
        num_cycling += number;
        while(number--){
            unit *cycle = new unit();
            cycle->type = "cycle";
            cycle->xi = position;
            cycle->yi = 15;
            cycle->de_type = 2;
            cycle->de_xnum = ui->cycling_length->text().toInt();
            cycle->de_ynum = ui->cycling_width->text().toInt();
            if(cycle->type == 1){
                cycle->length = cycle->de_xnum*de1_length_mm*1000/de_spacing_um + cycle->de_xnum-1;
                cycle->width = cycle->de_ynum*de1_length_mm*1000/de_spacing_um + cycle->de_ynum-1;
            }
            else{
                cycle->length = cycle->de_xnum*de2_length_mm*1000/de_spacing_um + cycle->de_xnum-1;
                cycle->width = cycle->de_ynum*de2_length_mm*1000/de_spacing_um + cycle->de_ynum-1;
            }
            cycle->color = cycling_color;
            ui->view->scene()->addItem(cycle);
            allunits.prepend(cycle);
            position += ui->merge_length->text().toInt() + 3;
            connect(cycle, SIGNAL(delete_this_item(unit *)), this, SLOT(delete_from_list(unit *)));
            num_de += (cycle->de_xnum + cycle->de_ynum - 2) * 2;
        }
        ui->num_cycling->setText(QString::number(num_cycling));
    }
}

//HEATER
void MainWindow::on_heater_create_clicked()
{
    if(linemode){
        WarningMessage("Cannot add components in LINE mode!");
    }
    else if(ui->heater_num->text().isEmpty())
    {
        WarningMessage("Missing Number");
    }
    else{
        EmptyMessage();
        int number = ui->heater_num->text().toInt();
        int position = 5;
        num_heater += number;
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
            num_de += 2;
        }
        ui->num_heater->setText(QString::number(num_heater));
    }
}
/////////////////////////////////////  SETTING  /////////////////////////////////////
void MainWindow::on_setting_btn_clicked()
{
    chip_setting *chip = new chip_setting(this);
    chip->setWindowTitle("Setting");
    connect(chip, SIGNAL(reset(chip_setting *)), this, SLOT(reset_setting(chip_setting *)));
    chip->show();
}

void MainWindow::clear_number_of_units(){
    //Number of units
    num_merge = 0;
    num_dispenser = 0;
    num_move = 0;
    num_cycling = 0;
    num_heater = 0;
    num_de = 0;
}
void MainWindow::reset_setting(chip_setting *new_chip)
{
    clear_number_of_units();
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
    line_width_um = new_chip->line_width_um;

    QGraphicsScene *newscene = CreateNewScene();
    graphicsscene *newlinescene = CreateLineScene();
    delete(ui->view->scene());
    allunits.clear();
    this->mainscene = newscene;
    this->linescene = newlinescene;
    ui->view->setScene(newscene);
}

/////////////////////////////////////  PREVIEW  /////////////////////////////////////
void MainWindow::on_preview_clicked(bool checked)
{

    if(checked){
        tempunits.clear();
        passunits.clear();
        errorunits.clear();
        //Create Checker_map
        checker_map = new int*[brick_xnum];
        for(int i = 0; i < brick_xnum; ++i){
            checker_map[i] = new int[brick_ynum];
            for(int j = 0; j < brick_ynum; j++)
                checker_map[i][j] = 0;
        }

        int border_brick = border_px / pix_per_brick;
        int fill_x, fill_y;

        //Fill Checker_map
        for(unit *item : allunits){
            bool jump = false;
            for(int x=0; x<item->length; x++){
                for(int y=0; y<item->width; y++){
                    fill_x = item->xi + x - border_brick;
                    fill_y = item->yi + y - border_brick;
                    if(fill_x >=0 && fill_y >=0 && fill_x < brick_xnum && fill_y < brick_ynum){
                        if(checker_map[fill_x][fill_y] == 0){
                            checker_map[fill_x][fill_y] = 1;
                        }
                        else{
                            errorunits << item;
                            jump = true;
                            continue;
                        }
                    }
                }
                if(jump)
                    continue;
            }
            if(jump)
                continue;
            else
                tempunits << item;
        }

        //Check every item. Make sure they all have neighbors
        for(unit *item : tempunits){
            int cross = 0;
            int start_x = item->xi - border_brick;
            int start_y = item->yi - border_brick;


            //Check if it is out of range
            if(start_x < 0 || start_y < 0 || start_x + item->length > brick_xnum || start_y + item->width > brick_ynum){
                errorunits << item;
                qDebug() << "out of range";
                continue;
            }

            //left
            start_x = item->xi - border_brick - 2;
            if(start_x >= 0){
                for(int a=0; a<item->width; a++){
                    if(start_y + a < brick_ynum){
                        if(checker_map[start_x][start_y + a])
                            cross++;
                    }
                }
            }
            //right
            start_x = item->xi - border_brick + item->length + 2;
            if(start_x < brick_xnum){
                for(int a=0; a<item->width; a++){
                    if(start_y + a < brick_ynum){
                        if(checker_map[start_x][start_y + a])
                            cross++;
                    }
                }
            }
            //top
            start_x = item->xi - border_brick;
            start_y = item->yi - border_brick - 2;
            if(start_y >=0){
                for(int a=0; a<item->length; a++){
                    if(start_x + a < brick_xnum){
                        if(checker_map[start_x + a][start_y])
                            cross++;
                    }
                }
            }
            //down
            start_y = item->yi - border_brick + item->width + 2;
            if(start_y < brick_ynum){
                for(int a=0; a<item->length; a++){
                    if(start_x + a < brick_xnum){
                        if(checker_map[start_x + a][start_y])
                            cross++;
                    }
                }
            }
            qDebug() << cross;

            if(cross >= 5){
                passunits << item;
            }
            else{
                errorunits << item;
            }
        }


        //QGraphicsScene * previewscene = new QGraphicsScene(0, 0, 600, 400, ui->view);
        previewscene = new QGraphicsScene(0, 0, 600, 400, ui->view);
        previewscene->setBackgroundBrush(Qt::white);
        graypen = QPen(Qt::black);
        redpen = QPen(Qt::black);
        linepen = QPen(Qt::black);
        nullitem = QBrush(Qt::black);

        OuterBorder(previewscene);
        ChipBorder(previewscene);

        EnableCreateUnit(false);
        for(unit *item : passunits){
            if(item->type == "move"){                   //Show detail components for "move"
                for(int i = 0; i < item->de_xnum; i++){
                    if(item->tilt == 90){
                        qDebug() << "tilted";
                        for(int i = 0; i < item->de_ynum; i++){
                            if(item->de_type == 1){
                                previewscene->addRect(item->xi*pix_per_brick,
                                           (item->yi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                           pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            } else {
                                previewscene->addRect(item->xi*pix_per_brick,
                                           (item->yi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                           pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            }
                        }
                    }
                    else{
                        for(int i = 0; i < item->de_xnum; i++){
                            if(item->de_type == 1){
                                previewscene->addRect((item->xi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           item->yi*pix_per_brick,
                                           pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                           pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            } else {
                                previewscene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           item->yi*pix_per_brick,
                                           pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                           pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            }
                        }
                    }
                }
            }
            else if(item->type == "cycle"){             //Show detail components for 'cycling"
                for(int i = 0; i < item->de_xnum; i++){
                    for(int j = 0; j < item->de_ynum; j++){
                        if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1) continue;
                            previewscene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      (item->yi+j*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                      pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                      redpen, nullitem);
                    }
                }
            }
            else
                previewscene->addRect(item->xi*pix_per_brick, item->yi*pix_per_brick, item->length*pix_per_brick, item->width*pix_per_brick, redpen, nullitem);
        }

//<<<<<<< Updated upstream
//        linepen.setWidth(line_width_pix);
//        for(line *turnline : linescene->alllines){
//            for(int i=0; i<turnline->segments; i++){
//                previewscene->addLine(turnline->x[i], turnline->y[i], turnline->x[i+1], turnline->y[i+1], linepen);
//            }
//        }
//=======

        linepen.setWidth(line_width_pix);        
        for(line *head : linescene->alllines){
            line *current_seg = head;
            while(current_seg->next != NULL){
                previewscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
                current_seg = current_seg->next;
            }
            previewscene->addLine(current_seg->x[0], current_seg->y[0], current_seg->x[1], current_seg->y[1], linepen);
        }


        nullitem = QBrush(Qt::red);
        for(unit *item : errorunits){
            if(item->type == "move"){                   //Show detail components for "move"
                for(int i = 0; i < item->de_xnum; i++){
                    if(item->tilt == 90){
                        qDebug() << "tilted";
                        for(int i = 0; i < item->de_ynum; i++){
                            if(item->de_type == 1){
                                previewscene->addRect(item->xi*pix_per_brick,
                                           (item->yi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                           pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            } else {
                                previewscene->addRect(item->xi*pix_per_brick,
                                           (item->yi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                           pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            }
                        }
                    }
                    else{
                        for(int i = 0; i < item->de_xnum; i++){
                            if(item->de_type == 1){
                                previewscene->addRect((item->xi+i*(de1_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           item->yi*pix_per_brick,
                                           pix_per_brick*de1_length_mm*1000/de_spacing_um,
                                           pix_per_brick*de1_width_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            } else {
                                previewscene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                           item->yi*pix_per_brick,
                                           pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                           pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                           redpen, nullitem);
                            }
                        }
                    }
                }
            }
            else if(item->type == "cycle"){             //Show detail components for 'cycling"
                for(int i = 0; i < item->de_xnum; i++){
                    for(int j = 0; j < item->de_ynum; j++){
                        if(i!=0 && i!=item->de_xnum-1 && j!=0 && j!=item->de_ynum-1) continue;
                            previewscene->addRect((item->xi+i*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      (item->yi+j*(de2_length_mm*1000/de_spacing_um + 1))*pix_per_brick,
                                      pix_per_brick*de2_length_mm*1000/de_spacing_um,
                                      pix_per_brick*de2_width_mm*1000/de_spacing_um,
                                      redpen, nullitem);
                    }
                }
            }
            else
                previewscene->addRect(item->xi*pix_per_brick, item->yi*pix_per_brick, item->length*pix_per_brick, item->width*pix_per_brick, redpen, nullitem);
        }
        QGraphicsTextItem *new_text_item = previewscene->addText(text_edit);
        new_text_item->setPos(brick_x_start, 400);

        ui->view->setScene(previewscene);
    }
    else{
        EnableCreateUnit(true);
        graypen = QPen(Qt::gray);
        redpen = QPen(Qt::red);
        linepen = QPen(Qt::gray);
        linepen.setWidth(line_width_pix);
        nullitem = QBrush(QColor(94, 94, 94, 54));
        if(!linemode){
            ui->view->setScene(mainscene);
        }
        else{
            ui->view->setScene(linescene);
        }
    }
}


/////////////////////////////////////  TEXT EDIT  /////////////////////////////////////
void MainWindow::on_text_enter_clicked()
{
    if(text_edited){
        delete text_item;
    }

    if(!ui->chip_name->text().isEmpty()) text_edit = ui->chip_name->text() + "\n";
    if(!ui->creator_name->text().isEmpty()) text_edit += ui->creator_name->text() + "\n";

    text_edit += ui->date_time->date().toString() + " ";            //顯示日期
    text_edit += ui->date_time->time().toString("hh:mm:ss") + "\n";  //顯示時間

    if(!ui->text_input->toPlainText().isEmpty()) text_edit += ui->text_input->toPlainText();
    QGraphicsTextItem *new_text_item = mainscene->addText(text_edit);
    new_text_item->setPos(brick_x_start, 400);
    text_item = new_text_item;
    text_edited = true;



}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    QDateTime *myDateTime = new QDateTime(QDateTime::currentDateTime());
    ui->date_time->setDateTime(QDateTime::currentDateTime());
}