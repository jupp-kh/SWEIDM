#include "ctdataset.h"
#include "QFile"
#include "cmath"
#include "QDebug"

CTDataset::CTDataset()
{
    // Platzhalter für den Datensatz
    orginalldata = new short[512*512*130];

    // Platzhalter für das Zwischenergebnis der 3D_image-Berechnung
    tiefenBuffer = new short[512*512];

    // Platzhalter für den gedrehten Datensatz
    m_pRotated = new short[512*512*512];

    // check whether the data are loaded
    loaded = false;
    m_pImageData = orginalldata;
    m_Rot.setIdentity();
    //hardgecodet
    width = 512;
    height = 512;
    end_width = 512;
    end_height = 512;
    start_width = 0;
    start_height = 0;
    layers = 130;
    //drehen(1,2,3);

}

CTDataset::~CTDataset(){
    delete [] orginalldata;
    delete [] tiefenBuffer;
    delete [] m_pRotated;
}

/**
  get methode für die private variabel m_pImageData
  @return den Datensatz
*/
short* CTDataset::data(){
    return m_pImageData;
}
/**
 nimme die ursprünglische date */
void CTDataset::rotateBack(){
    m_pImageData = orginalldata;
    layers = 130;
}
/**
  get methode für die private variabel depthbuffer
  @return den Tiefenkarte
*/
short* CTDataset::depthBuffer(){
    return tiefenBuffer;
}
/**
  zu loaden von image data
  @param path of the file
  @return fehler code. \n 0 : erfolgreich \n 1 : überlauf \n 2 : file not existet.
*/
int CTDataset::load(QString imagePath){
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);
    if (!bFileOpen){return 2;}
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char*)orginalldata, 512*512*130*sizeof(short));
    if (iFileSize != iNumberBytesRead){return 1;}
    dataFile.close();
    loaded = true;
    return 0;
}
/**
 * berechne die fenestrirung damit jeder gewebe angezeigt werden kann
 * @param HU_value grau wert in dem ursprünglich 0 - 4096
 * @param startvalue start value für die fenestrirung
 * @param windowWidth das intervall der fenestrirung
 * @param greyValue referenc zu die grau wert 0-255
 * @return fehler code.\n 0 : erfolgreich\n 1 : HU_value falsch \n 2 : start value falsch \n 3 : windowidth falsch.
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
void CTDataset::corpping(const int start_x, const int start_y, const int end_x,const int end_y  ){
    start_width = start_x;
    start_height = start_y;
    end_width = end_x;
    end_height = end_y;

}
/** Mit dieser Funktion wird die Tiefe berechnet, damit das Bild in 3D angezeigt wird.
 *Sie geht durch den Datensatz und schaut, bei welchen Ebenen der Schwellenwert erreicht wird
 *und speichert die Tiefe in dem Array depthBuffer
 * @param threashold ist das schwellenwert
 * @return fehler code.\n 0 : erfolgreich\n 1 : no data \n 2 : threashold bound fehler
*/
int CTDataset::calculateDepthBuffer(const int &threashold){
    if(!loaded) return 1;
    if(threashold < -1024 || threashold >3071) return 2;
    for(int index = 0; index < width*height ; index++){
        tiefenBuffer[index] = layers - 1 ;
        for(int x = layers -1 ;x > 0; x--){
            if(( m_pImageData[index+(x*width*height)]>=threashold)){
                // wertebereich 0 - layers
                tiefenBuffer[index] = layers - 1 -x;
                break;
            }
        }
    }
    return  0;
}
/**
 * Diese Funktion dient der Berechnung der 3D-Ansicht aus mehreren Bildern
 * @param shadedBuffer arrey hat das 3d visualisiertes bild
 * @return fehler code\n 0 : erfolgreich
*/
int CTDataset::renderDepthBuffer(short *shadedBuffer){
    /// t_* ist die tiefe
    int index,t_y,t_x;
    for(int y = 0 ; y <height; y++){
        for(int x = 0; x<width ;x++){
            index = y*height + x;
            //if condition to check the rand pixel im tiefenBuffer spalte 0,511
            if (x>= 1 && x<=510){
                t_x = std::abs(tiefenBuffer[index+1] - tiefenBuffer[index-1]);
            }else{
                t_x = tiefenBuffer[index];
            }
            //if condition to check the rand pixel im tiefenBuffer zeile 0,511
            if (y>= 1 && y<=510){
                t_y = std::abs(tiefenBuffer[index + height] - tiefenBuffer[index - height]);
            }else {
                t_y= tiefenBuffer[index];
            }
            //s_x*s_y = 4; 16 = (s_x*s_y)^2
            shadedBuffer[index] = 255* (4/ sqrt(pow(2*t_x,2)+pow(2*t_y,2)+ 16));
       }
    }
    return  0;
}

void CTDataset::rotate(const int &threashold){
    std::fill_n(m_pRotated, width*height*512, -1024);
    Eigen::Vector3d a;
    Eigen::Vector3d center(255.5,255.5,64.5);
    for(int z = 0 ; z < layers ; z++){
        for(int y = start_height ; y < end_height ; y++){
            for(int x = start_width ; x < end_width ; x++){
                a.x() = x;
                a.y() = y;
                a.z() = z;
                a =  m_Rot * (a - center) + center  ;
                if (a.x() < 0 || a.x() > 511) continue;
                if (a.y() < 0 || a.y() > 511) continue;
                if (a.y() < 0 || a.y() > 511) continue;

                m_pRotated [(int)a.x()+ 512*(int)a.y() + 512*512*(int)a.z()  ] =  orginalldata[512*512*z + 512*y + x];

            }
        }
    }

    layers = 512;
    m_pImageData = m_pRotated;
    calculateDepthBuffer(threashold);
}

/**
this funktion is to update the rotation matrix an store the value untel we use it
@param xAngle rotation on the x axis
@param yAngle rotation on the y axis
@param zAngle rotation on the z axis
*/

void CTDataset::updateDrehMatrix(const int& x,const int& y,const int& z){
     int xAngle = x;
     int yAngle = y;
     int zAngle = z;
     if(xAngle > 180) xAngle  = 360 - xAngle;
     if(yAngle > 180) yAngle  = 360 - yAngle;
     if(zAngle > 180) zAngle  = 360 - yAngle;
     m_Rot =  Eigen::AngleAxisd(xAngle/180.*M_PI, Eigen::Vector3d::UnitX()) * m_Rot;
     m_Rot =  Eigen::AngleAxisd(yAngle/180.*M_PI, Eigen::Vector3d::UnitY()) * m_Rot;
     m_Rot =  Eigen::AngleAxisd(zAngle/180.*M_PI, Eigen::Vector3d::UnitZ()) * m_Rot;
}
