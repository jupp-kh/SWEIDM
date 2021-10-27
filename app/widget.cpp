#include "widget.h"
#include "ui_widget.h"
#include "QFile"
#include "QElapsedTimer"
#include "QDebug"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    image_3d = new short[512*512*130];
    ui->setupUi(this);
    connect(ui->pushButton_8bit, SIGNAL(clicked()),this,SLOT(Malebild()));
    connect(ui->pushButton_12bit, SIGNAL(clicked()),this,SLOT(load_12bit()));
    connect(ui->pushButton_3d, SIGNAL(clicked()),this,SLOT(load_3d()));
    connect(ui->horizontalSlide_start,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingStart(int)));
    connect(ui->horizontalSlider_width,SIGNAL(valueChanged(int)),this,SLOT(updatedWindowingWidth(int)));
    connect(ui->verticalSlider_schichten,SIGNAL(valueChanged(int)),this,SLOT(updatedschichtnummer(int)));
}

Widget::~Widget()
{
    delete ui;
    delete [] image_3d;
}

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
    // einen roten Pixel
    // POSITION, rot,grün,blau
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "./", "Raw Image Files (*.raw)");
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");return;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char*)image_3d, 512*512*sizeof(short));
    if (iFileSize != iNumberBytesRead){QMessageBox::critical(this, "ACHTUNG", "Überlauf");return;}
    dataFile.close();
    updateSliceView();
}

void Widget::load_3d(){
    QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "./", "Raw Image Files (*.raw)");
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");return;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char*)image_3d, 512*512*130*sizeof(short));
    if (iFileSize != iNumberBytesRead){QMessageBox::critical(this, "ACHTUNG", "Überlauf");return;}
    dataFile.close();
    updateSchichtView();
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
    updateSchichtView();
}
void Widget::updateSchichtView(){
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    int x, y, iGrauwert ;
    for (int index = 0; index<512*512; index++) {
        windowing(image_3d[index+(ui->verticalSlider_schichten->value()*512*512)],
                              ui->horizontalSlide_start->value(),
                              ui->horizontalSlider_width->value(),
                              iGrauwert);

        x = index % 512;
        y = index / 512;
        image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
    }
    ui->label->setPixmap(QPixmap::fromImage(image));
}
void Widget::updateSliceView(){
    QElapsedTimer timer;
    timer.start();
    QImage image(512,512,QImage::Format_RGB32);
    image.fill(qRgb(0,0,0));
    int x, y, iGrauwert ;
    for (int index = 0; index<512*512; index++) {
        windowing(image_3d[index+(ui->verticalSlider_schichten->value()*512*512)],
                              ui->horizontalSlide_start->value(),
                              ui->horizontalSlider_width->value(),
                              iGrauwert);

        x = index % 512;
        y = index / 512;
        image.setPixel(x,y ,qRgb(iGrauwert, iGrauwert, iGrauwert));
    }
    ui->label->setPixmap(QPixmap::fromImage(image));
    qDebug()<<timer.nsecsElapsed();
}
int Widget::windowing( int HU_value,  int startValue, int windowWidth, int &greyValue){
    // prüfe parm auf out_of_range
    if (HU_value <-1024 || HU_value> 3071) return 1;
    if (startValue< -1024 || startValue > 3071) return 2;
    if (windowWidth <1 || windowWidth> 4095) return  3;

    if (HU_value< startValue){
        greyValue = 0;
        return 0;
    }
    if (HU_value > startValue+windowWidth){
        greyValue = 255;
        return 0;
    }
    greyValue = (HU_value-startValue)*(255.0/windowWidth);
    return 0;
}
