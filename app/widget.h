#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <ctdataset.h>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE
/**
    *hallo world
*/
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    Ui::Widget *ui;
    short* shadedBuffer;
   // int windowing(int HU_value, int startValue, int windowWidth, int &greyValue);
    bool segmentierung( int HU_value, int schwellenwert);
    void updateSliceView();
    int calculateDepthBuffer(short* inputData,int width, int height, int layers, int threashold,short* depthBuffer);
    CTDataset dataset;
    bool tiefenBufferEx;
    int xMousePress, yMousePress, xMouseRelease,yMouseRelease ;

private slots:
    //void Malebild();
    //void load_12bit();
    void load_3d();
    void updatedWindowingStart(int value);
    void updatedWindowingWidth(int value);
    void updatedschichtnummer(int value);
    void updatedschwellenwert(int value);
    void updatedAxis();
    void cropImage();
    void rotateBack();
    void render3D();
    void erzeugeTestData();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // WIDGET_H
