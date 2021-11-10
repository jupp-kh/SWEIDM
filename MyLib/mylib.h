#ifndef MYLIB_H
#define MYLIB_H

#include "MyLib_global.h"


class MYLIB_EXPORT MyLib
{
public:
    MyLib();
    static int windowing(int HU_value,int startValue,int windowWidth,int &greyValue);
    static int calculateDepthBuffer(short* inputData,int width, int height, int layers, int threashold,short* depthBuffer);
    static int renderDepthBuffer(const short*depthBuffer,int width,int height,short*shadedBufer);
};

#endif // MYLIB_H
