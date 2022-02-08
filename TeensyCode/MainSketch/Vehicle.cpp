#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>
#include <SD.h>
#include <time.h>
#include "Vehicle.h"


// Initialize display
void Vehicle::BeginDisplay(){
  GD.begin(0);
}


// Set start time to current time, so runTime == 0
void Vehicle::ResetTimer(){
  this->startTime = time(0);
}



int Vehicle::GetRunTime(){
    this->runTime = time(0) - this->startTime;
}


// TODO: change to collect, store, and return speed
int Vehicle::GetSpeedMPH(){
  return this->speedMPH;
}



// TODO: include all data in write pass
// Write all data to SD card with time stamp
bool Vehicle::WriteToSD(){
  SD.begin(254);
  File dataFile = SD.open("VehicleData.txt", FILE_WRITE);
  if(dataFile){ // 254==BUILTIN_SDCARD, set 
    dataFile.print((int)time(0));
    dataFile.print(",");
    dataFile.print(this->runTime);
    dataFile.print(",");
    dataFile.print(this->fuel);
    dataFile.print(",");
    dataFile.print(this->rpm);
    dataFile.print(",");
    dataFile.print(this->speedMPH);
    dataFile.print(",");
    dataFile.print(this->tempAmb);
    dataFile.print(",");
    dataFile.print(this->tempCVT);
    dataFile.print(",");
    dataFile.print(this->accelerometer[0]);
    dataFile.print(",");
    dataFile.print(this->accelerometer[1]);
    dataFile.print(",");
    dataFile.print(this->accelerometer[2]);
    dataFile.print("\n");
    dataFile.close();
    return true;    
  }
  dataFile.close();
  return false;
}



// TODO: Get ambient temperature
int Vehicle::GetTempAmb(){
  return this->tempAmb;
}



// Gets CVT temperature from sensor and returns as int
// TODO: Get ambient temperature
int Vehicle::GetTempCVT(){
  return this->tempCVT;
}



// Collects, updates, and returns engine RPM
// TODO: Get engine rpm
int Vehicle::GetRPM(){
  return this->rpm;
}



// Returns fuel level between 0 and 100
// TODO: Get fuel level (0-100%)
int Vehicle::GetFuelLevel(){
  return this->fuel;
}



// TODO: Include all desired data on display pass
// Refresh display with new data
void Vehicle::Display(){
  // Formatting
  GD.ClearColorRGB(0, 0, 40); // Set background color
  GD.Clear();
  GD.ColorRGB(0xffffff); // Set text/element color

  // Display engine RPM
  String rpmString = (String)this->rpm;
  char rpmChar[10];
  rpmString.toCharArray(rpmChar, 10);
  GD.cmd_gauge(GD.w/2, GD.h/2, 100, OPT_NOBACK, 20, 100, rpmChar, 750);
  GD.cmd_text(GD.w/2, GD.h/2+50, 31, OPT_CENTER, rpmChar);
  GD.cmd_text(GD.w/2, GD.h/2+80, 29, OPT_CENTER, "RPM");

  //Display fuel level
  GD.cmd_progress(GD.w-35, 70, 25, GD.h-90, OPT_CENTER, this->fuel, 100);
  GD.cmd_text(GD.w-23, 30, 31, OPT_CENTER, "F");

  GD.swap();
  
/* DISPLAY EXAMPLES:

  Dial measurement:
  GD.cmd_gauge(<x position>, <y position>, <radius>, <centering options>, <minor ticks>, <major ticks>, <value>, <range>);

  Plain text: (only ASCII characters 0-127 supported, idk why)
  GD.cmd_text(<x position>, <y position>, <text size (kind of), use 31>, <centering options>, <char textArray[]>);

  Progress bar:
  GD.cmd_progress(<xpos>, <ypos>, <width>, <height>, <centering options>, <value>, <range>); 
 */
}










