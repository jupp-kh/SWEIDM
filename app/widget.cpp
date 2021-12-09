


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
    ui->setupUi(this);
    connect(ui->pushButton_render3d, SIGNAL(clicked()),this,SLOT(render3D()));
    connect(ui->pushButton_load3d, SIGNAL(clicked()),this,SLOT(load_3d()));
    connect(ui->pushButton_test, SIGNAL(clicked()),this,SLOT(erzeugeTestData()));
    connect(ui->pushButton_back, SIGNAL(clicked()),this,SLOT(rotateBack()));
    connect(ui->pushButton_crop, SIGNAL(clicked()),this,SLOT(cropImage()));
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
        xMousePress = localPos3D.x();
        ui->label_y->setText("y_Pos: " + QString::number(localPos3D.y()));
        yMousePress = localPos3D.y();
        ui->label_z->setText("z_Pos: " + QString::number(dataset.depthBuffer()[localPos3D.y() * 512 + localPos3D.x()]+1));
    }
    // prüfe ob die kurse sich im lable2D  befindet
    if(ui->label_2D->rect().contains(localPos2D)){
        ui->label_x->setText("x_Pos: " + QString::number(localPos2D.x()));
        ui->label_y->setText("y_Pos: " + QString::number(localPos2D.y()));
        ui->label_z->setText("z_Pos: " + QString::number(ui->verticalSlider_schichten->value()+1));
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
        xMouseRelease = localPos3D.x();
        yMouseRelease = localPos3D.y();
    }
    ui->label_crop->setText("cropbetween:"+ QString::number(xMousePress) +"__"+ QString::number(yMousePress) +"\n" +QString::number(xMouseRelease)+"__" + QString::number(yMouseRelease) );

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
    ui->label_schicht->setText("schicht:" + QString::number(value + 1 ));
    updateSliceView();
}
void Widget::updatedschwellenwert(int value){
    ui->label_schwellenwert->setText("render3D:" + QString::number(value));
    updateSliceView();
}
void Widget::updatedAxis(){
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
    ui->label_2D->setPixmap(QPixmap::fromImage(image));
   // qDebug()<<timer.nsecsElapsed();
}


bool Widget::segmentierung( int HU_value,int schwellenwert){
    if (HU_value > schwellenwert) return true;
    return false ;
}
void Widget::cropImage(){
    if(xMousePress >= 0 && yMousePress >= 0 && xMousePress < 512 && yMousePress <512 && xMouseRelease >= 0 && yMouseRelease >= 0 && xMouseRelease < 512 && yMouseRelease <512 ){
       dataset.corpping(std::min(xMousePress,xMouseRelease),std::min(yMousePress,yMouseRelease),std::max(xMousePress,xMouseRelease),std::max(xMousePress,xMouseRelease));
    }
}

// in diese funktion wird mit ein zeiger auf die orginal daten gezeigt
void Widget::rotateBack(){
    ui->verticalSlider_schichten->setMaximum(129);
    dataset.rotateBack();
}

void Widget::render3D(){
    //rechne den tiefen karte
    if (ui->checkBox_rotation->isChecked()){
        ui->verticalSlider_schichten->setMaximum(511);
        dataset.rotate(ui->horizontalSlider_schwellenwert->value());
    }else{
        dataset.calculateDepthBuffer(ui->horizontalSlider_schwellenwert->value());
    }
    tiefenBufferEx = true;
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



