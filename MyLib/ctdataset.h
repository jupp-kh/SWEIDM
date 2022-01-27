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
    short* getschablone();

    ~CTDataset();
    static int windowing(int HU_value,int startValue,int windowWidth,int &greyValue);
    int calculateDepthBuffer(const int &iThreshold);
    int renderDepthBuffer(short*shadedBuffer);
    void rotate(const int &threashold);
    void updateDrehMatrix(const int& xAngle, const int& yAngle, const int& zAngle);
    void orginallstate();
    void corpping(const int start_x, const int start_y, const int end_x,const int end_y  );
    void undocrop();
    void filterBild();
    void bohren(Eigen::Vector3d start, Eigen::Vector3d end, int bohrDM);
    void deletplan();
    Eigen::Vector3d  schablone(Eigen::Vector3d start, Eigen::Vector3d end, const int &iThreshold );
    void displaySchablone();
    Eigen::Matrix3d get_rotatationMatrix();

private:
    short* m_pImageData;
    int width,height,layers;
    short* tiefenBuffer;
    short* orginalldata;
    short* schablone_data;
    bool* maske;
    bool loaded;
    short*  m_pRotated;
    Eigen::Matrix3d m_Rot;
    Eigen::Vector4d croop;

};

#endif // CTDATASET_H
