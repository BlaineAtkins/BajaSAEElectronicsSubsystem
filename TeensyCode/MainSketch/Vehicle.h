#ifndef VEHICLE_H
#define VEHICLE_H

//for BNO055
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* TODO:
    -Confirm gps, accelerometer, brake pressure data types
    -Add orientation variable + related functions
    -Function TODOs and descriptions
    -Block out pins/code for display panel buttons
    -Complete this todo list lol
*/

class Vehicle{
private:

    void BeginDisplay();

    float tempAmb = 0;
    int tempCVT = 0;
    int runTime = 0;
    int fuel = 100; // 0-100
    //double gpsPos[2] = {0, 0}; // x, y?
    //double accelerometer[3] = {0, 0, 0}; // x, y, z
    double brakePressure = 0;

    //for GPS
    double gpsLat = 0;
    double gpsLng = 0;
    double gpsSpeed = 0;
    int gpsSecond = 0;
    int gpsMinute = 0;
    int gpsHour = 0;
    int gpsDay = 0;
    int gpsMonth = 0;
    int gpsYear = 0;
    double gpsCourse = 0;

    //for 9dof
    double rawAccelx = 0;
    double rawAccely = 0;
    double rawAccelz = 0;
    double linAccelx = 0;
    double linAccely = 0;
    double linAccelz = 0;
    double gravityx = 0;
    double gravityy = 0;
    double gravityz = 0;
    double orientationx = 0;
    double orientationy = 0;
    double orientationz = 0;
    double magx = 0;
    double magy = 0;
    double magz = 0;
    double gyrox = 0;
    double gyroy = 0;
    double gyroz = 0;
    int8_t boxTemp = 0;

    unsigned long cycleTimer=0;
    int timePerCycle;
    
public:
    Vehicle(){ // Constructor
        this->BeginDisplay(); // Can't call GD.begin() from header >:(

        if(!bno.begin()){
          Serial.print("Could not detect BNO055 IMU. Please make sure it is plugged in to the main board");
        }
        bno.setExtCrystalUse(true);
    }
    ~Vehicle(){}; // Destructor

    //IMU
    Adafruit_BNO055 bno = Adafruit_BNO055(55);

    int refTime = 0;
    float speedMPH = 0;
    int rpm = 500; // 0-1000
    bool darkMode = true;
    bool verboseMode = false;

    
    bool WriteToSD();
    int GetSpeedMPH();
    int GetFuelLevel();
    int GetTempAmb();
    int GetTempCVT();
    int GetRPM();
    void GetGPSData();
    void DisplayGPSOnSerial();
    void Get9dofData();
    int GetCycleTime();
    void WriteToRadio();
//    void AppendChar(char* char1, char* char2, int l1, int l2);
    void Display();
    void DisplayVerbose();
};
#endif
