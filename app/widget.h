#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    short* image_3d;
    int windowing(int HU_value, int startValue, int windowWidth, int &greyValue);
    void updateSliceView();



private slots:
    void Malebild();
    void load_12bit();
    void load_3d();
    void updatedWindowingStart(int value);
    void updatedWindowingWidth(int value);
    void updatedschichtnummer(int value);
};
#endif // WIDGET_H
