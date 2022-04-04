#ifndef VEHICLE_H
#define VEHICLE_H

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
    double gpsPos[2] = {0, 0}; // x, y?
    double accelerometer[3] = {0, 0, 0}; // x, y, z
    double brakePressure = 0;
    // TODO: orientation

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
    
public:
    Vehicle(){ // Constructor
        this->BeginDisplay(); // Can't call GD.begin() from header >:(
    }
    ~Vehicle(){}; // Destructor

    int speedMPH = 0;

    bool WriteToSD();
    int GetSpeedMPH();
    int GetFuelLevel();
    int GetTempAmb();
    int GetTempCVT();
    int GetRPM();
    void GetGPSData();
    void DisplayGPSOnSerial();
//    void AppendChar(char* char1, char* char2, int l1, int l2);
    void Display();
};
#endif
