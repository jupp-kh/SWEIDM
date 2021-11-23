#ifndef CTDATASET_H
#define CTDATASET_H
#include <QFile>
#include <MyLib_global.h>
class MYLIB_EXPORT CTDataset
{

public:
    CTDataset();
    int load(QString imagePath);
    short* data();


    ~CTDataset();
    static int windowing(int HU_value,int startValue,int windowWidth,int &greyValue);
    int calculateDepthBuffer(const int&iThreshold);
    int renderDepthBuffer(short*shadedBuffer);


private:
    short* m_pImageData;
    int width,height,layers;
    short* depthBuffer;
};

#endif // CTDATASET_H
