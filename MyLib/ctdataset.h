#ifndef CTDATASET_H
#define CTDATASET_H
#include <QFile>
#include <MyLib_global.h>
#include "Eigen/Core"
#include "Eigen/Dense"

class MYLIB_EXPORT CTDataset
{

public:
    CTDataset();
    int load(QString imagePath);
    short* data();
    short* depthBuffer();

    ~CTDataset();
    static int windowing(int HU_value,int startValue,int windowWidth,int &greyValue);
    int calculateDepthBuffer(const int &iThreshold);
    int renderDepthBuffer(short*shadedBuffer);
    void rotate(const int &threashold);
    void updateDrehMatrix(const int& xAngle, const int& yAngle, const int& zAngle);
    void rotateBack();
    void corpping(const int start_x, const int start_y, const int end_x,const int end_y  );

private:
    short* m_pImageData;
    int width,height,layers,start_width,start_height,end_width, end_height;
    short* tiefenBuffer;
    short* orginalldata;
    bool* maske;
    bool loaded;
    short*  m_pRotated;
    Eigen::Matrix3d m_Rot;

};

#endif // CTDATASET_H
