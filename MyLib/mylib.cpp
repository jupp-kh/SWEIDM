#include "mylib.h"
#include "cmath"

MyLib::MyLib()
{

}

int MyLib::windowing( int HU_value,  int startValue, int windowWidth, int &greyValue){
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
    greyValue = std::roundf((HU_value-startValue)*(255.0/windowWidth));
    return 0;
}
/* width,height,layers bestimmt der größer des Datensatz
 * threashold ist das schwellenwert
 * diese function ist zu berechnen von die tiefe damit das bild 3d
 * angezeigt wird.
 * der geht die daten satz durch und shaut nach an welche layers ist die schwellenwert zu treffen
 * und speichert die tife in die array depthBuffer
*/
int MyLib::calculateDepthBuffer(short* inputData,int width, int height, int layers, int threashold,short* depthBuffer){
    for(int index = 0; index < width*height ; index++){
        depthBuffer[index] = layers - 1 ;
        for(int x = layers -1 ;x > 0; x--){
            if((inputData[index+(x*width*height)]>=threashold)){
                // wertebereich 0 - layers
                depthBuffer[index] = layers - 1 -x;
                break;
            }
        }
    }
    return  0;
}
int MyLib::renderDepthBuffer(const short *depthBuffer, int width, int height, short *shadedBuffer){
    int index,t_y,t_x;
    for(int y = 0 ; y <height; y++){
        for(int x = 0; x<width ;x++){
            index = y*height + x;
            //if condition to check the rand pixel im depthBuffer spalte 0,511
            if (x>= 1 && x<=510){
                t_x = std::abs(depthBuffer[index+1] - depthBuffer[index-1]);
            }else{
                t_x = depthBuffer[index];
            }
            //if condition to check the rand pixel im depthBuffer zeile 0,511
            if (y>= 1 && y<=510){
                t_y = std::abs(depthBuffer[index + height] - depthBuffer[index - height]);
            }else {
                t_y= depthBuffer[index];
            }
            //s_x*s_y = 4; 16 = (s_x*s_y)^2
            shadedBuffer[index] = 255* (4/ sqrt(pow(2*t_x,2)+pow(2*t_y,2)+ 16));
       }
    }
}
