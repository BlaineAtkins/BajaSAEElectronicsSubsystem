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
    int rpm = 500; // 0-1000
    int fuel = 100; // 0-100
    int runTime = 0;
    int startTime = 0;
    //double gpsPos[2] = {0, 0}; // x, y?
    //double accelerometer[3] = {0, 0, 0}; // x, y, z
    double brakePressure = 0;

    //for GPS
    double gpsLat;
    double gpsLng;
    double gpsSpeed;
    int gpsSecond;
    int gpsMinute;
    int gpsHour;
    int gpsDay;
    int gpsMonth;
    int gpsYear;
    double gpsCourse;

    //for 9dof
    double rawAccelx;
    double rawAccely;
    double rawAccelz;
    double linAccelx;
    double linAccely;
    double linAccelz;
    double gravityx;
    double gravityy;
    double gravityz;
    double orientationx;
    double orientationy;
    double orientationz;
    double magx;
    double magy;
    double magz;
    double gyrox;
    double gyroy;
    double gyroz;
    int8_t boxTemp;

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

    int speedMPH = 0;

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
//    void AppendChar(char* char1, char* char2, int l1, int l2);
    void Display();
};
#endif
