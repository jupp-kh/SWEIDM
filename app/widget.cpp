
#include "widget.h"
#include "ui_widget.h"
#include "QFile"
#include "QElapsedTimer"
#include "QDebug"
#include "mylib.h"
#include "cmath"
/**
 * hallow world
*/
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    tiefenBufferEx = false;
    shadedBuffer = new short[512*512];

    // die confirm bool variable ist für sceduling die immage anzeigen
    confirm_start = false;
    confirm_end =false;
    confirmcrop = false;
    display_schablone = false;
    start.setZero();
    end.setZero();
    ui->setupUi(this);
    connect(ui->pushButton_render3d, SIGNAL(clicked()),this,SLOT(render3D()));
    connect(ui->pushButton_load3d, SIGNAL(clicked()),this,SLOT(load_3d()));
    connect(ui->pushButton_test, SIGNAL(clicked()),this,SLOT(erzeugeTestData()));
    connect(ui->pushButton_back, SIGNAL(clicked()),this,SLOT(orginallstate()));
    connect(ui->pushButton_crop, SIGNAL(clicked()),this,SLOT(cropImage()));
    connect(ui->pushButton_undocrop, SIGNAL(clicked()),this,SLOT(undocrop()));
    connect(ui->pushButton_croparea, SIGNAL(clicked()),this,SLOT(cropArea()));
    connect(ui->pushButton_saveplan, SIGNAL(clicked()),this,SLOT(saveplan()));
    connect(ui->pushButton_bohrP, SIGNAL(clicked()),this,SLOT(bohren()));
    connect(ui->pushButton_hideP, SIGNAL(clicked()),this,SLOT(deletplan()));
    connect(ui->pushButton_start, SIGNAL(clicked()),this,SLOT(confirmStart()));
    connect(ui->pushButton_end, SIGNAL(clicked()),this,SLOT(confirmEnd()));
    connect(ui->pushButton_schablone, SIGNAL(clicked()),this,SLOT(schablone()));
    connect(ui->pushButton_saveschablone, SIGNAL(clicked()),this,SLOT(saveschablone_raw()));
    connect(ui->pushButton_displayschablone, SIGNAL(clicked()),this,SLOT(displaySchablone()));


    connect(ui->horizontalSlide_start,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingStart(int)));
    connect(ui->horizontalSlider_width,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingWidth(int)));
    connect(ui->horizontalSlider_schwellenwert,SIGNAL(valueChanged(int)),this,SLOT(updatedschwellenwert(int)));
    connect(ui->verticalSlider_schichten,SIGNAL(valueChanged(int)),this,SLOT(updatedschichtnummer(int)));
    connect(ui->horizontalSlider_xaxis,SIGNAL(valueChanged(int)),this,SLOT(updatedAxis()));
    connect(ui->horizontalSlider_yaxis,SIGNAL(valueChanged(int)),this,SLOT(updatedAxis()));
    connect(ui->horizontalSlider_zaxis,SIGNAL(valueChanged(int)),this,SLOT(updatedAxis()));

}

Widget::~Widget()
{
    delete ui;
    delete [] shadedBuffer;
}


void Widget::mousePressEvent(QMouseEvent *event){
    // kurse position(x,y) in die qt fienster
    QPoint globalPos = event->pos();
    event->button();
    // kurse position(x,y) im 3d_label
    QPoint localPos3D = ui->label_3D->mapFromParent(globalPos);

    // kurse position(x,y) im 2d_label
    QPoint localPos2D = ui->label_2D->mapFromParent(globalPos);

    // prüfe ob die kurse sich im label3D befindet
    if (ui->label_3D->rect().contains(localPos3D) && tiefenBufferEx){
        ui->label_x->setText("x_Pos: " + QString::number(localPos3D.x()));
        MousePress3D.x() = localPos3D.x();
        ui->label_y->setText("y_Pos: " + QString::number(localPos3D.y()));
        MousePress3D.y() = localPos3D.y();

        if (dataset.get_rotatationMatrix().isIdentity()){
            ui->label_z->setText("z_Pos: " + QString::number( 130 - dataset.depthBuffer()[localPos3D.y() * 512 + localPos3D.x()]));
            MousePress3D.z() = 130 -  dataset.depthBuffer()[localPos3D.y() * 512 + localPos3D.x()] ;
        }else{
            ui->label_z->setText("z_Pos: " + QString::number(512 - dataset.depthBuffer()[localPos3D.y() * 512 + localPos3D.x()]));
            MousePress3D.z() = 512 -  dataset.depthBuffer()[localPos3D.y() * 512 + localPos3D.x()] ;
        }
    }
    // prüfe ob die kurse sich im lable2D  befindet
    if(ui->label_2D->rect().contains(localPos2D)){
        ui->label_x->setText("x_Pos: " + QString::number(localPos2D.x()));
        MousePress2D.x() = localPos2D.x();
        ui->label_y->setText("y_Pos: " + QString::number(localPos2D.y()));
        MousePress2D.y() = localPos2D.y();
        ui->label_z->setText("z_Pos: " + QString::number(ui->verticalSlider_schichten->value()+1));
        MousePress2D.z() = ui->verticalSlider_schichten->value()+1;
    }

}


void Widget::mouseReleaseEvent(QMouseEvent *event){
    // kurse position(x,y) in die qt fienster
    QPoint globalPos = event->pos();
    event->button();
    // kurse position(x,y) im 3d_label
    QPoint localPos3D = ui->label_3D->mapFromParent(globalPos);


    // prüfe ob die kurse sich im label3D befindet
    if (ui->label_3D->rect().contains(localPos3D) && tiefenBufferEx){
        MouseRelease3D.x() = localPos3D.x();
        MouseRelease3D.y() = localPos3D.y();
    }
    ui->label_crop->setText("cropbetween:\n x: "+ QString::number(MousePress3D.x()) +" __ "+ QString::number(MousePress3D.y()) +"\n y: " +QString::number(MouseRelease3D.y())+" __ " + QString::number(MouseRelease3D.y()) );

}


void Widget::erzeugeTestData(){
    short * testdataset = dataset.data();
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
                for (int index = 0; index < 130; ++index ) {
                    testdataset[(index *512*512) +(x+(y*512))] = -1024;
                    if(index == 125 && x != 1 && y != 1 ){
                        testdataset[(index * 512*512)+(x+(512*y))] = 3071;
                    }
            }

        }
    }
    QFile dataFile( QFileDialog::getSaveFileName(this, "save Image", "./", "Raw Image Files (*.raw)"));
    dataFile.open(QIODevice::WriteOnly);
    dataFile.write((char*)testdataset, 512*512*130*sizeof(short));
    dataFile.close();
}
void Widget::load_3d(){

    // open File Dialog to select dataset
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "./", "Raw Image Files (*.raw)");

    //try to load
    int iErrorCode = dataset.load(imagePath);


    if (iErrorCode == 0){
        updateSliceView();
    }
    else if (iErrorCode == 1 )
        QMessageBox::critical(this, "ACHTUNG", "File Not Found");
    else
        QMessageBox::critical(this, "ACHTUNG", "inconsisten File size");
}
void Widget::updatedWindowingStart(int value){
    ui->label_start->setText("Start: " + QString::number(value));
    updateSliceView();
}
void Widget::updatedWindowingWidth(int value){
    ui->label_width->setText("Width: " + QString::number(value));
    updateSliceView();
}
void Widget::updatedschichtnummer(int value){
    ui->label_schicht->setText("schicht:" + QString::number(value + 1 ));
    updateSliceView();
}
void Widget::updatedschwellenwert(int value){
    ui->label_schwellenwert->setText("Schwellenwert:" + QString::number(value));
    updateSliceView();
}
void Widget::   updatedAxis(){
    ui->label_xaxis->setText("x_axis: " + QString::number(ui->horizontalSlider_xaxis->value()));
    ui->label_yaxis->setText("y_axis: " + QString::number(ui->horizontalSlider_yaxis->value()));
    ui->label_zaxis->setText("z_axis: " + QString::number(ui->horizontalSlider_zaxis->value()));
    dataset.updateDrehMatrix(ui->horizontalSlider_xaxis->value(),ui->horizontalSlider_yaxis->value(),ui->horizontalSlider_zaxis->value());
}

void Widget::updateSliceView(){
    //QElapsedTimer timer;
    // timer.start();
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    int x, y, iGrauwert;
    for (int index = 0; index<512*512; index++) {
         y = index / 512;
         x = index % 512;
         if(display_schablone){
             if(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)] == 0 ){
               image.setPixel(x,y ,qRgb(0,255,0));
               continue;
             }
             if(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)] == 1 ){
               image.setPixel(x,y ,qRgb(255,0,0));
               continue;
             }
         }
         if(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)]==10000 ){
           image.setPixel(x,y ,qRgb(0,0,255));
           continue;
         }
         if  (CTDataset::windowing(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)],
                              ui->horizontalSlide_start->value(),
                              ui->horizontalSlider_width->value(),
                              iGrauwert)!= 0) qDebug() <<"falshe wert empfangen";
         // rechne index vom bild
         if(segmentierung(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)],ui->horizontalSlider_schwellenwert->value())){
             image.setPixel(x,y,qRgb(255,0,0));
             continue;
         }


        image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
    }
    ui->label_2D->setPixmap(QPixmap::fromImage(image));
   // qDebug()<<timer.nsecsElapsed();
}


bool Widget::segmentierung( int HU_value,int schwellenwert){
    if (HU_value > schwellenwert) return true;
    return false ;
}


// in diese funktion wird mit ein zeiger auf die orginal daten gezeigt
void Widget::orginallstate(){
    ui->verticalSlider_schichten->setMaximum(129);
    ui->horizontalSlider_xaxis->setValue(0);
    ui->horizontalSlider_yaxis->setValue(0);
    ui->horizontalSlider_zaxis->setValue(0);
    confirm_start = false;
    confirm_end =false;
    confirmcrop = false;
    display_schablone = false;
    dataset.orginallstate();
    render3D();

}

void Widget::render3D(){
    //rechne den tiefen karte
    QElapsedTimer all;
    all.start();
    if(ui->horizontalSlider_xaxis->value() != 0 || ui->horizontalSlider_yaxis->value() != 0 || ui->horizontalSlider_zaxis->value() !=0 ){
        ui->verticalSlider_schichten->setMaximum(511);
    }
    dataset.rotate(ui->horizontalSlider_schwellenwert->value());
    updateSliceView();
    tiefenBufferEx = true;
    dataset.renderDepthBuffer(shadedBuffer);
    QElapsedTimer timer;
    timer.start();
    displayImage3D();

    qDebug()<<"image:"<<timer.nsecsElapsed();
    qDebug()<<"rander time :"<<all.nsecsElapsed();
}
void Widget::bohren(){
    if (confirm_start && confirm_end){
        dataset.bohren(start ,end, ui->spinBox_bohrDM->value());
        updateSliceView();
    }else{
          QMessageBox::critical(this, "ACHTUNG", "please confirm the start and the end");
    }


}

void Widget::saveplan(){
    QString fileName= QFileDialog::getSaveFileName(this, tr("Speichern der Planung"), tr("./Planung.txt"), tr("Text Files (*.txt)"));
    if(fileName != "") {
        QFile file(QFileInfo(fileName).absoluteFilePath());
        if(!file.open(QIODevice::WriteOnly)){
            QMessageBox::critical(this, tr("Error"), tr("Failedtosave file"));
            return; //Aborted
        }
        //All ok -save data
        Eigen::Vector3d center(512/2,512/2,130/2);
        MousePress2D = dataset.get_rotatationMatrix()*(MousePress2D - center ) +center;
        MousePress3D = dataset.get_rotatationMatrix()*(MousePress3D - center ) +center;
        Eigen::Vector3d bohrvector = end -start ;
        int n = std::sqrt(std::pow(bohrvector.x(),2)+std::pow(bohrvector.y(),2)+std::pow(bohrvector.z(),2));
        QTextStream out(&file);
            out << "start: " << MousePress3D.x() << " , "<< MousePress3D.y() << Qt::endl;
            out << "end: " << MousePress3D.x() << " , " << MousePress2D.y() << Qt::endl;
            out << "Durschmesser: " << ui->spinBox_bohrDM->value()<< Qt::endl;
            out << "bohrlange: " << n<< Qt::endl;
            out << "schablone:" << Qt::endl;
            for (int z = 0; z <40 ;z++) {
                for (int y = 0; y<40 ;y++) {
                    for (int x = 0; x <40 ;x++) {
                        out << dataset.getschablone()[x + y*40 + z*40*40] << " ";
                    }
                    out << Qt::endl;
                }
                out << Qt::endl;
            }


            file.close();}
}
void Widget::cropImage(){
    if(confirmcrop){
       dataset.corpping(std::min(MousePress3D.x(),MouseRelease3D.x()),
                        std::min(MousePress3D.y(),MouseRelease3D.y()),
                        std::max(MousePress3D.x(),MouseRelease3D.x()),
                        std::max(MousePress3D.y(),MouseRelease3D.y()));
       render3D();
       confirmcrop = false;

    }else{
        QMessageBox::critical(this, "ACHTUNG", "please select a abox from the 3d_bild with mouse then confirm the area ");
    }
}

void Widget::undocrop(){
    dataset.undocrop();
    render3D();
    confirmcrop = false;
}
void Widget::cropArea(){
     if(MousePress3D.x() >= 0 && MousePress3D.y() >= 0
        && MousePress3D.x() < 512 && MousePress3D.y() <512
        && MouseRelease3D.x() >= 0 && MouseRelease3D.y() >= 0
       && MouseRelease3D.x() < 512 && MouseRelease3D.y() <512 ){
     confirmcrop = true;
     displayImage3D();
     }
    else{
          QMessageBox::critical(this, "ACHTUNG", "please select a abox from the 3d_bild with mouse ");
     }
}


void Widget::deletplan(){
    confirm_start = false;
    confirm_end = false;
    display_schablone = false;
    render3D();
}
void Widget::confirmStart(){
    start = MousePress3D;
    confirm_start = true;
    displayImage3D();
}

void Widget::confirmEnd(){
    end = MousePress2D;
    confirm_end = true;
    displayImage3D();
}

void Widget::displayImage3D(){
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    int x,y,iGrauwert;
    for (int index = 0;index <512*512 ;index++) {
        // rechne index vom bild
        iGrauwert =shadedBuffer[index];
        y = index / 512;
        x = index % 512;
    image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
}
    if (confirmcrop){
        int startx = std::min(MousePress3D.x(),MouseRelease3D.x());
        int starty = std::min(MousePress3D.y(),MouseRelease3D.y());
        int endx = std::max(MousePress3D.x(),MouseRelease3D.x());
        int endy = std::max(MousePress3D.y(),MouseRelease3D.y());
        for (int x = startx ;x < endx ;x++) {
            image.setPixel(x,starty ,qRgb(0, 0, 255));
            image.setPixel(x,endy ,qRgb(0, 0, 255));
        }
        for (int y = starty ;y < endy ;y++) {
            image.setPixel(startx,y ,qRgb(0, 0, 255));
            image.setPixel(endx,y ,qRgb(0, 0, 255));
        }
    }
    if (confirm_start){
        for (int i = 0 ; i < 5; i++) {
                image.setPixel(start.x()+i,start.y()+i,qRgb(255, 0, 0));
                image.setPixel(start.x()-i,start.y()+i,qRgb(255, 0, 0));
                image.setPixel(start.x()+i,start.y()-i,qRgb(255, 0, 0));
                image.setPixel(start.x()-i,start.y()-i,qRgb(255, 0, 0));
        }
    }
    if(confirm_end){
        for (int i = 0 ; i < 5; i++) {
                image.setPixel(end.x()+i,end.y()+i,qRgb(0,255,0));
                image.setPixel(end.x()-i,end.y()+i,qRgb(0,255,0));
                image.setPixel(end.x()+i,end.y()-i,qRgb(0,255,0));
                image.setPixel(end.x()-i,end.y()-i,qRgb(0,255,0));
        }
    }
ui->label_3D->setPixmap(QPixmap::fromImage(image));
}
void Widget::saveschablone_raw(){
    QFile dataFile( QFileDialog::getSaveFileName(this, "save Image", "./", "Raw Image Files (*.raw)"));
    dataFile.open(QIODevice::WriteOnly);
    dataFile.write((char*)dataset.getschablone(), 40*40*40*sizeof(short));
    dataFile.close();
}
void Widget::displaySchablone(){
    display_schablone = true;
    ui->verticalSlider_schichten->setMaximum(511);
    dataset.displaySchablone();
    updateSliceView();
    dataset.renderDepthBuffer(shadedBuffer);
    displayImage3D();

}

void Widget::schablone(){
    if(confirm_start && confirm_end){
        qDebug()<<"in schablone";
        Eigen::Vector3d s = start;
        start = dataset.schablone(start,end,ui->horizontalSlider_schwellenwert->value(),ui->spinBox_bohrDM->value());

        updateSliceView();
        confirm_end = false;
        tiefenBufferEx = true;
        dataset.renderDepthBuffer(shadedBuffer);
        displayImage3D();
        start = s;
        confirm_start =false;


    }else{
         QMessageBox::critical(this, "ACHTUNG", "please confirm the start and end point");
    }

}
