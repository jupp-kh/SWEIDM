


#include "widget.h"
#include "ui_widget.h"
#include "QFile"
#include "QElapsedTimer"
#include "QDebug"
#include "mylib.h"
/**
 * hallow world
*/
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    shadedBuffer = new short[512*512];
    ui->setupUi(this);
    //connect(ui->pushButton_8bit, SIGNAL(clicked()),this,SLOT(Malebild()));
    //connect(ui->pushButton_12bit, SIGNAL(clicked()),this,SLOT(load_12bit()));
    connect(ui->pushButton_render3d, SIGNAL(clicked()),this,SLOT(render3D()));
    connect(ui->pushButton_load3d, SIGNAL(clicked()),this,SLOT(load_3d()));
    connect(ui->pushButton_test, SIGNAL(clicked()),this,SLOT(erzeugeTestData()));
    connect(ui->horizontalSlide_start,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingStart(int)));
    connect(ui->horizontalSlider_width,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingWidth(int)));
    connect(ui->horizontalSlider_schwellenwert,SIGNAL(valueChanged(int)),this,SLOT(updatedschwellenwert(int)));
    connect(ui->verticalSlider_schichten,SIGNAL(valueChanged(int)),this,SLOT(updatedschichtnummer(int)));

}

Widget::~Widget()
{
    delete ui;
    delete [] shadedBuffer;
}
/*
void Widget::Malebild(){
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    // einen roten Pixel
    // POSITION, rot,grün,blau
    char imageData[512*512];
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "./", "Raw Image Files (*.raw)");
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");return;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read(imageData, 512*512);
    if (iFileSize != iNumberBytesRead){QMessageBox::critical(this, "ACHTUNG","speicher überlauf");return;}
    dataFile.close();
    for (int index = 0; index<512*512; index++) {
        int iGrauwert = imageData[index];
        int x = index % 512;
        int y = index / 512;
        image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
    }
    ui->label->setPixmap(QPixmap::fromImage(image));
}

void Widget::load_12bit(){

    // open File Dialog to select dataset
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "./", "Raw Image Files (*.raw)");
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");return;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char*)dataset.data(), 512*512*sizeof(short));
    if (iFileSize != iNumberBytesRead){QMessageBox::critical(this, "ACHTUNG", "Überlauf");return;}
    dataFile.close();
    updateSliceView();
}*/

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
    qDebug()<<imagePath;
    //try to load
    int iErrorCode = dataset.load(imagePath);

    if (iErrorCode == 0)
        updateSliceView();
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
    ui->label_schicht->setText("schicht:" + QString::number(value));
    updateSliceView();
}
void Widget::updatedschwellenwert(int value){
    ui->label_schwellenwert->setText("render3D:" + QString::number(value));
    updateSliceView();
}

void Widget::updateSliceView(){
    //QElapsedTimer timer;
    // timer.start();
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    int x, y, iGrauwert;
    for (int index = 0; index<512*512; index++) {
         if  (CTDataset::windowing(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)],
                              ui->horizontalSlide_start->value(),
                              ui->horizontalSlider_width->value(),
                              iGrauwert)!= 0) qDebug() <<"falshe wert empfangen";
         // rechne index vom bild
         y = index / 512;
         x = index % 512;
         if(segmentierung(dataset.data()[index+(ui->verticalSlider_schichten->value()*512*512)],ui->horizontalSlider_schwellenwert->value())){
             image.setPixel(x,y,qRgb(255,0,0));
             continue;
         }


        image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
    }
    ui->label->setPixmap(QPixmap::fromImage(image));
   // qDebug()<<timer.nsecsElapsed();
}


bool Widget::segmentierung( int HU_value,int schwellenwert){
    if (HU_value > schwellenwert) return true;
    return false ;
}


void Widget::render3D(){
    //rechne den tiefen karte
    dataset.calculateDepthBuffer(ui->horizontalSlider_schwellenwert->value());
    dataset.renderDepthBuffer(shadedBuffer);
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
ui->label_3D->setPixmap(QPixmap::fromImage(image));
}



