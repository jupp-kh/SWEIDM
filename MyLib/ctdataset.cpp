#include "ctdataset.h"
#include "QFile"
#include "cmath"
#include "QDebug"

CTDataset::CTDataset()
{
    // Platzhalter für den Datensatz
    m_pImageData = new short[512*512*130];
    // Platzhalter für das Zwischenergebnis der 3D_image-Berechnung
    depthBuffer = new short[512*512];

    //hardgecodet
    width = 512;
    height = 512;
    layers = 130;
}

CTDataset::~CTDataset(){
    delete [] m_pImageData;
    delete [] depthBuffer;
}

/**
  zu loaden von image data
  @param path of the file
  @return fehler code 1 = überlauf, 2 = file not existet
*/
int CTDataset::load(QString imagePath){
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){return 2;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char*)m_pImageData, 512*512*130*sizeof(short));
    if (iFileSize != iNumberBytesRead){return 1;}
    dataFile.close();
    return 0;
}
/**
  get methode für die private variabel m_pImageData
  @return den Datensatz
*/
short* CTDataset::data(){
    return m_pImageData;
}
/**
 * berechne die fenestrirung damit jeder gewebe erstelt wird
 * @param HU_value grau wert in dem ursprünglich 0 - 4096
 * @param startvalue start value für die fenestrirung
 * @param windowWidth das intervall der fenestrirung
 * @param greyValue referenc zu die grau wert 0-255
 * @return fehler code
*/
int CTDataset::windowing( int HU_value,  int startValue, int windowWidth, int &greyValue){
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
/** Mit dieser Funktion wird die Tiefe berechnet, damit das Bild in 3D angezeigt wird.
 *Sie geht durch den Datensatz und schaut, bei welchen Ebenen der Schwellenwert erreicht wird
 *und speichert die Tiefe in dem Array depthBuffer
 * @param threashold ist das schwellenwert
*/
int CTDataset::calculateDepthBuffer(const int &threashold){
    for(int index = 0; index < width*height ; index++){
        depthBuffer[index] = layers - 1 ;
        for(int x = layers -1 ;x > 0; x--){
            if(( m_pImageData[index+(x*width*height)]>=threashold)){
                // wertebereich 0 - layers
                depthBuffer[index] = layers - 1 -x;
                break;
            }
        }
    }
    return  0;
}
/**
 * Diese Funktion dient der Berechnung der 3D-Ansicht aus mehreren Bildern
 * @param shadedBuffer arrey hat das 3d visualisiertes bild
*/
int CTDataset::renderDepthBuffer(short *shadedBuffer){
    /// t_* ist die tiefe
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
    return  0;
}
