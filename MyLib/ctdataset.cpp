#include "ctdataset.h"
#include "QFile"
#include "cmath"
#include "QDebug"
#include "QElapsedTimer"

CTDataset::CTDataset()
{
    // Platzhalter für den Datensatz
    orginalldata = new short[512*512*130];

    // Platzhalter für das Zwischenergebnis der 3D_image-Berechnung
    tiefenBuffer = new short[512*512];

    // Platzhalter für den gedrehten Datensatz
    m_pRotated = new short[512*512*512];
    //
    schablone_data = new short[40*40*40];
    maske = new bool[512*512];

    std::fill_n(maske,512*512,true);

    // check whether the data are loaded
    loaded = false;
    m_pImageData = orginalldata;
    m_Rot.setIdentity();
    //hardgecodet
    width = 512;
    height = 512;
    croop[0] = 0;
    croop[1] = 0;
    croop[2] = 512;
    croop[3] = 512;
    layers = 130;
    //drehen(1,2,3);

}

CTDataset::~CTDataset(){
    delete [] orginalldata;
    delete [] tiefenBuffer;
    delete [] m_pRotated;
    delete [] maske;
    delete [] schablone_data;
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
void CTDataset::orginallstate(){
    m_pImageData = orginalldata;
    layers = 130;
    std::fill_n(maske, width*height,true);
    croop[0] = 0;
    croop[1] = 0;
    croop[2] = 512;
    croop[3] = 512;
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
    filterBild();
    loaded = true;
    return 0;
}
/**
 dieser funktion zum filteren das angabe\n
 er schaut die benachbarten voxel und set the voxel auf dem mittlern wert
*/
void CTDataset::filterBild(){

    for(int z = 0; z<layers; z++){
        for(int y = 1 ; y<511; y++){
            for(int x = 1; x<511; x++){
                std::vector<short> vec(9);
                vec[0] = orginalldata[(z*height* width)+(width*(y-1)) + (x-1) ];
                vec[1] = orginalldata[(z*height* width)+(width*y) + (x-1) ];
                vec[2] = orginalldata[(z*height* width)+(width*(y+1)) + (x-1) ];
                vec[3] = orginalldata[(z*height* width)+(width*(y-1)) + x];
                vec[4] = orginalldata[(z*height* width)+(width*y) + x ];
                vec[5] = orginalldata[(z*height* width)+(width*(y+1)) + x ];
                vec[6] = orginalldata[(z*height* width)+(width*(y-1)) + (x+1) ];
                vec[7] = orginalldata[(z*height* width)+(width*y) + (x+1) ];
                vec[8] = orginalldata[(z*height* width)+(width*(y+1)) + (x+1) ];
                std::sort(vec.begin(), vec.end());
                orginalldata[(z*height* width)+(width*y) + x ] = vec[4];
            }
        }
    }

}
/**
 * berechne die fenestrirung damit jeder gewebe angezeigt werden kann
 * @param HU_value grau wert in dem ursprünglich 0 - 4096
 * @param startvalue start value für die fenestrirung [-1024 - 3071]
 * @param windowWidth das intervall der fenestrirung [1 - 4095]
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


/**
dieser funktion setzt die cordinaten der cropping für weiter benuzung*/
void CTDataset::corpping(const int start_x, const int start_y, const int end_x,const int end_y  ){

    croop[0] = start_x;
    croop[1] = start_y;
    croop[2] = end_x;
    croop[3] = end_y;

}
/**
setzt die cropping zurück
*/
void CTDataset::undocrop(){
    croop[0] = 0;
    croop[1] = 0;
    croop[2] = 512;
    croop[3] = 512;
}
/** Mit dieser Funktion wird die Tiefe berechnet, damit das Bild in 3D angezeigt wird.
 *Sie geht durch den Datensatz und schaut, bei welchen Ebenen der Schwellenwert erreicht wird
 *und speichert die Tiefe in dem Array depthBuffer
 * @param threashold ist das schwellenwert -1024 -3071
 * @return fehler code.\n 0 : erfolgreich\n 1 : no data \n 2 : threashold bound fehler
*/
int CTDataset::calculateDepthBuffer(const int &threashold){
    QElapsedTimer timer;
    timer.start();
    if(!loaded) return 1;
    if(threashold < -1024 || threashold >3071) return 2;
    int x;
    for(int index = 0; index < width*height ; index++){
        x = layers -1;
        if(maske[index]){

            while(  ( m_pImageData[index+(x*width*height)]<threashold)&& x > 0 ){
               x--;
            }
        }
        tiefenBuffer[index] = layers - 1 -x;
    }
    qDebug()<<"calculatedepthbuffer:"<<timer.nsecsElapsed();
    return  0;
}
/**
 * Diese Funktion dient der Berechnung der 3D-Ansicht aus mehreren Bildern
 * @param shadedBuffer arrey hat das 3d visualisiertes bild
 * @return fehler code\n 0 : erfolgreich
*/
int CTDataset::renderDepthBuffer(short *shadedBuffer){
    // t_* ist die tiefe
    QElapsedTimer timer;
    timer.start();
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

    qDebug()<<"randerDepthBuffer"<<timer.nsecsElapsed();
    return  0;

}
/**
   * @param shadedBuffer arrey hat das 3d visualisiertes bild
   * in dieser funktion wird das bild rotiert um den axien nach beliebten winkel
*/

void CTDataset::rotate(const int &threashold){
    QElapsedTimer timer;
    timer.start();
    std::fill_n(m_pRotated, width*height*512, -1024);
    //  set index von vor schleife falls das bild ausgeschnitten wird
    int start_width = croop[0];
    int start_height = croop[1];
    int end_width = croop[2];
    int end_height = croop[3];
    Eigen::Vector3d a;
    Eigen::Vector3d center(start_width + (end_width - start_width)/2,start_height + (end_height-start_height)/2,layers/2);
    Eigen::Vector3d rotatV = m_Rot * (-center) + center;
    for(int z = 0 ; z < layers ; z++){
        for(int y = start_height ; y < end_height ; y++){
            for(int x = start_width ; x < end_width ; x++){
                a.x() = x;
                a.y() = y;
                a.z() = z;
                a =  m_Rot * a + rotatV  ;
                if (a.x() < 0 || a.x() > 511) continue;
                if (a.y() < 0 || a.y() > 511) continue;
                if (a.z() < 0 || a.z() > 511) continue;
                m_pRotated [(int)a.x()+ 512*(int)a.y() + 512*512*(int)a.z()  ] =  orginalldata[512*512*z + 512*y + x];
                maske[x+ 512 * y] = true;
            }
        }
    }
    if(!m_Rot.isIdentity()){
         layers = 512;
    }
    m_pImageData = m_pRotated;
    qDebug()<<"rotate:"<<timer.nsecsElapsed();
    calculateDepthBuffer(threashold);
    layers = 130;
}

/**
this funktion is to update the rotation matrix an store the value untel we use it
@param xAngle rotation on the x axis
@param yAngle rotation on the y axis
@param zAngle rotation on the z axis
*/

void CTDataset::updateDrehMatrix(const int& x,const int& y,const int& z){
     m_Rot =  Eigen::AngleAxisd(x/180.*M_PI, Eigen::Vector3d::UnitX());
     m_Rot =  Eigen::AngleAxisd(y/180.*M_PI, Eigen::Vector3d::UnitY()) * m_Rot;
     m_Rot =  Eigen::AngleAxisd(z/180.*M_PI, Eigen::Vector3d::UnitZ()) * m_Rot;
}
/**
 geter ffür rotation matrix
*/
Eigen::Matrix3d CTDataset::get_rotatationMatrix(){
    return m_Rot;
}

/**
  @param start start punkt der bohrvector
  @param end   end punkt der bohrvector
  @param bohrDM bohrdurchmesse
  in dieser funktion wird in dem data ein exploziert wert gesetzt \n
 um spater die Böre lange und durchmesse zu wisualisieren
*/
void CTDataset::bohren(Eigen::Vector3d start, Eigen::Vector3d end, int bohrDM){

   Eigen::Vector3d bohrvector = end - start;
   Eigen::Vector3d bohrindex;
   float n = std::sqrt(std::pow(bohrvector.x(),2)+std::pow(bohrvector.y(),2)+std::pow(bohrvector.z(),2));
   Eigen::Vector3d nomiert = bohrvector/n;
   for(int i = 0 ; i < n ; i++){
        bohrindex = start + (i*nomiert);
         for(int x = (int)bohrindex.x()- bohrDM ; x <= (int)bohrindex.x()+bohrDM ; x++){
             for(int y = (int)bohrindex.y()- bohrDM ; y <= (int)bohrindex.y()+bohrDM ; y++){
                 for(int z = (int)bohrindex.z()- bohrDM ;z <= (int)bohrindex.z()+bohrDM ; z++){
                    if(std::pow((bohrindex.x() - x ),2) + std::pow((bohrindex.y() - y ),2) + std::pow((bohrindex.z() - z ),2) <=std::pow( bohrDM,2)){
                        if (x < 0 || x > 511) continue;
                        if (y < 0 || y > 511) continue;
                        if (z < 0 || z > 511) continue;
                        m_pImageData[x+ 512*y + 512*512*z] = 10000;
                    }
                 }
             }
         }

   }
}



/**
  dieser funktion löscht die modifiziert data und set die data zu orginal
*/
void CTDataset::deletplan(){
    m_pImageData = orginalldata;
}

/**
  @param start start punkt der bohrvector
  @param end end punkt der bohrvector
  @param threashold ist das schwellenwert -1024 -3071
dieser funktion rechnet die schablone surface

*/
Eigen::Vector3d CTDataset::schablone(Eigen::Vector3d start, Eigen::Vector3d end, const int &threashold, int bohrDM){
    Eigen::Vector3d center(512/ 2 , 512/ 2, 130/2);

    std::fill_n(schablone_data,40*40*40,1);
    Eigen::Vector3d bohrvector =   ( start - end ) ;

    // save m_Rot; aktuelle zustand
    Eigen::Matrix3d rot;
    rot = m_Rot;
    // lange in zy ebene
    float n_zy = std::sqrt(std::pow(bohrvector.y(),2)+std::pow(bohrvector.z(),2));



    // set x to 0 in dem die vectro gedret um y
    // die ecke hier ist zwischen bohrvector und das ebene ZY
    float yzangle = -atan(bohrvector.x()/bohrvector.z());
    // ecke in 2D zwischen behrvector und z axis
    float zangle =  -acos(bohrvector.z()/n_zy) ;

    // die rotation matrix zu visualisiern
    m_Rot = Eigen::AngleAxisd(yzangle, Eigen::Vector3d::UnitY())* m_Rot;
    m_Rot =  Eigen::AngleAxisd(zangle, Eigen::Vector3d::UnitX()) * m_Rot;


    start = Eigen::AngleAxisd(yzangle, Eigen::Vector3d::UnitY())*Eigen::AngleAxisd(zangle, Eigen::Vector3d::UnitX())* (start -center ) + center ;
    end = Eigen::AngleAxisd(yzangle, Eigen::Vector3d::UnitY())*Eigen::AngleAxisd(zangle, Eigen::Vector3d::UnitX())* (start -center ) + center ;
    bohrvector = end -start;
    float n = std::sqrt(std::pow(bohrvector.x(),2)+std::pow(bohrvector.y(),2)+std::pow(bohrvector.z(),2));
    Eigen::Vector3d nomiert = bohrvector/n;
    Eigen::Vector3d bohr_index;
    qDebug()<<yzangle<<zangle;

    // nach rotation wird die routation matrix zurück gesetzt für weiter benuzung;
    rotate(threashold);
    bohren(start, end , bohrDM);
    m_Rot = rot;
    //versuch
    for (int x = -20; x < 20; x++) {
        for(int y = -20 ; y < 20 ; y++){
            for(int z = 0 ; z < 10 ; z++){
                Eigen::Vector3d xy(x,y,0);
                bohr_index =  start + (z*nomiert) + xy ;

                if(schablone_data[(x+20) + 40 *(y +20) + (z+30) *40*40] == 0) break;
                if ((int)bohr_index.x() < 0 || (int)bohr_index.x()> 511) continue;
                if ((int)bohr_index.y() < 0 || (int)bohr_index.y() > 511) continue;
                if ((int)bohr_index.z() < 0 ||(int)bohr_index.z() > 511) continue;
                if(m_pImageData[(int)bohr_index.x()+ 512*(int)bohr_index.y() + 512*512*(int)bohr_index.z()] >= threashold ){
                    qDebug()<<m_pImageData[(int)bohr_index.x()+ 512*(int)bohr_index.y() + 512*512*(int)bohr_index.z()];
                    int j_1 = z+30;
                    if (m_pImageData[(int)bohr_index.x()+ 512*(int)bohr_index.y() + 512*512*(int)bohr_index.z()] == 10000) j_1 = 0;
                    for(int j = j_1; j < 40; j++){


                        schablone_data[(x+20) + 40 *(y +20) + j *40*40] = 0;
                    }
                    break;
                }
            }
        }
    }





    return start;

}

/**
geter für schablone datat*/
short* CTDataset::getschablone(){
    return schablone_data;
}

/**
diser funktion schreibt die schablone daten in großen array zum viszalisieren mit dem gleichen software;
*/
void CTDataset::displaySchablone(){
    m_pImageData = m_pRotated;
    int s_x,s_y,s_z;
    for (int x = 0; x < 512; ++x) {
        for (int y = 0; y < 512; ++y) {
            for (int z = 0; z < 512; ++z) {
                s_x = x/512.0 * 40;
                s_y = y/512.0 * 40;
                s_z = z/512.0 * 40 ;
                  m_pImageData[x+ 512*y + 512*512*z] = schablone_data[s_x + 40 * s_y + s_z *40*40];


            }
        }
    }
    calculateDepthBuffer(1);
}
