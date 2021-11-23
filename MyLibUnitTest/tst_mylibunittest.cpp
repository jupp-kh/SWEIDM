#include <QString>
#include <QtTest>
#include "mylib.h"
#include <algorithm>
#include "ctdataset.h"

class MyLibUnitTest : public QObject
{
    Q_OBJECT

public:
    MyLibUnitTest();

private Q_SLOTS:
   void windowingTest();

};

MyLibUnitTest::MyLibUnitTest()
{
}

/**
 * @brief Test cases for MyLib::windowing(...)
 */
void MyLibUnitTest::windowingTest()
{
    // equivalence classes, input based:
    // HU values: -1024 <= HU < 3072, three separate input params
    // where <HU start> must be within this domain, but also
    // <HU start> + <windowing width> must be in the domain
    // also, <windowing width> must be postive
    // (return int ref cannot be tested, references also cannot be nullptrs)

    // equivalence classes, output based:
    // linear function, therefore test two sample points

    //windowing( int HU_value,  int startValue, int windowWidth, int &greyValue)
    // VALID case 1: testing clean zero for bottom HU boundary
    int returnedVal = 1;
    int returnCode  = 0;
    returnCode = CTDataset::windowing(  -34,   -34,   100, returnedVal);
    QVERIFY2(returnCode  == 0, "returns an error although input is valid");
    QVERIFY2(returnedVal == 0, "windowing function lower bound");

    // VALID case 2: testing center of windowed domain
    returnCode  =  0;
    returnedVal = -1;
    returnCode = CTDataset::windowing(   50,     0,   100, returnedVal);
    QVERIFY2(returnCode  == 0, "returns an error although input is valid");
    QVERIFY2(returnedVal == 128, qPrintable( QString("windowing function medium value, was %1 instead of 128").arg(returnedVal) ) );

    // INVALID case 1: HU start too low
    returnedVal = 0;
    returnCode  = 0;
    returnCode =CTDataset::windowing(  100, -1500,  1000, returnedVal);
    QVERIFY2(returnCode == 2, "Incorrect error code returned although windowing start value was <-1024");

    // INVALID case 2: HU input too low
    returnedVal = 0;
    returnCode  = 0;
    returnCode = CTDataset::windowing(-4100, -1000,  2000, returnedVal);
    QVERIFY2(returnCode == 1, "No error code returned although input HU value was <-1024");

    // INVALID case 3: HU input too high
    returnedVal = 0;
    returnCode  = 0;
    returnCode = CTDataset::windowing( 3100,  -100,  2000, returnedVal);
    QVERIFY2(returnCode == 1, "No error code returned although input HU value was >3071");

    // INVALID case 4: HU start too high
    returnedVal = 0;
    returnCode  = 0;
    returnCode = CTDataset::windowing(  100,  3072,  100, returnedVal);
    QVERIFY2(returnCode == 2, "No error code returned although window start > 3071");

    // INVALID case 5: HU window width negative
    returnedVal = 0;
    returnCode  = 0;
    returnCode = CTDataset::windowing(  200,    50,  -300, returnedVal);
    QVERIFY2(returnCode == 3, "No error code returned although negative window width.");

    // INVALID case 6: HU window width too high (> 4095)
    returnedVal = 0;
    returnCode  = 0;
    returnCode = CTDataset::windowing(  100,  1024,  4096, returnedVal);
    QVERIFY2(returnCode == 3, "No error code returned although window width > 4095");

    // VALID case 7: HU input greater than HU start + HU window widht
    returnedVal = 1;
    returnCode  = 0;
    returnCode = CTDataset::windowing(  71 ,   -30,   100, returnedVal);
    QVERIFY2(returnCode  == 0, "returns an error although input is valid");
    QVERIFY2(returnedVal == 255, "windowing function upper bound");

    // VALID case 7: HU input greater than HU start + HU window widht
    returnedVal = 1;
    returnCode  = 0;
    returnCode = CTDataset::windowing(  -71 ,   -30,   100, returnedVal);
    QVERIFY2(returnCode  == 0, "returns an error although input is valid");
    QVERIFY2(returnedVal == 0, "windowing function lower bound");

}


QTEST_APPLESS_MAIN(MyLibUnitTest)

#include "tst_mylibunittest.moc"
