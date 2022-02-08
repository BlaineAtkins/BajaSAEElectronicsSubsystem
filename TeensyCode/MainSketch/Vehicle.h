#ifndef VEHICLE_H
#define VEHICLE_H

class Vehicle{
private:
    int tempAmbient = 0;
    int tempCVT = 0;
    int rpm = 0; // 0-1000
    int fuel = 100; // 0-100
    int startTime = 0;
    int runTime = 0;
    double gpsPos[2] = {0, 0}; // x, y?
    double accelerometer[3] = {0, 0, 0}; // x, y, z
    double brakePressure = 0;
    // TODO: orientation

public:
    Vehicle(){}; // Constructor
    ~Vehicle(){}; // Destructor

    int GetRunTime();
    int GetFuelLevel();
    int GetTempAmb();
    int GetTempCVT();
    int GetRPM();
    void Display();


}
#endif