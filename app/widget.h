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
    void displayImage3D();
    int calculateDepthBuffer(short* inputData,int width, int height, int layers, int threashold,short* depthBuffer);
    CTDataset dataset;
    bool tiefenBufferEx,confirm_start, confirm_end,confirmcrop, display_schablone;
    Eigen::Vector3d MousePress3D;
    Eigen::Vector3d MouseRelease3D;
    Eigen::Vector3d MousePress2D;
    Eigen::Vector3d start;
    Eigen::Vector3d end;


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
    void undocrop();
    void orginallstate();
    void render3D();
    void saveplan();
    void erzeugeTestData();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void bohren();
    void deletplan();
    void schablone();
    void confirmStart();
    void confirmEnd();
    void cropArea();
    void saveschablone_raw();
    void displaySchablone();

};

#endif // WIDGET_H
