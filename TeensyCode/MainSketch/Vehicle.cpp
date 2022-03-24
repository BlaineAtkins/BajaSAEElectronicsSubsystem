#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>
#include <SD.h>
#include "Vehicle.h"

//for ambient temp
#include <OneWire.h> //download from https://github.com/PaulStoffregen/OneWire for teensy compatible
#include <DallasTemperature.h> //download on step 3 from https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
#define ONE_WIRE_BUS 31
OneWire oneWire(ONE_WIRE_BUS); //this bus can be used for other OneWire devices too
DallasTemperature sensors(&oneWire);


// Initialize display
void Vehicle::BeginDisplay(){
  Serial.println("If you get stuck here, the display is disconnected. Please connect display to boot.");
  GD.begin(0);
  //for ambient temperature
  sensors.begin();
  sensors.setWaitForConversion(false); //don't wait ~750 ms for temp to return new value
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
    dataFile.print((int)(millis()/1000));
    dataFile.print(",");
    dataFile.print(millis()/1000.0);
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
  sensors.requestTemperatures();
  
  float rawTemp=sensors.getTempCByIndex(0); //0 is first IC on wire
  //Serial.print(rawTemp);
  float rawHigh=97.00; //value from sensor at boiling
  float rawLow=0; //value from sensor at melting
  float referenceHigh=100; //reference value at boiling (based on altitude)
  float referenceLow=0; //reference value at melting (based on altitude)

  this->tempAmb = (((rawTemp-rawLow)*(referenceHigh-referenceLow))/(rawHigh-rawLow))+referenceLow;

  return this->tempAmb;
}



// Gets CVT temperature from sensor and returns as int
// TODO: Get ambient temperature
int Vehicle::GetTempCVT(){
  float voltage = 3.3/1024*analogRead(41);
  this->tempCVT = voltage/3*450-70; //equation from manufacturer website and verified
  Serial.println(this->tempCVT);
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
  int textColor = 0xffffff; // White text
  textColor = 0; // Black text

  GD.ClearColorRGB(0, 0, 40); // Set background color
  GD.ClearColorRGB(0xffffff); // override to white background
  GD.Clear();
  GD.ColorRGB(textColor); // Set text/element color

  // Display run time
  int timeMin = (int)(millis()/60000.0);
  String timeString = (String)timeMin + "min";
  char timeChar[100];
  timeString.toCharArray(timeChar, 100);
  GD.cmd_text(GD.w-130, 30, 31, OPT_CENTER, "Run Time:");
  GD.cmd_text(GD.w-130, 70, 31, OPT_CENTER, timeChar);

  // Display CVT temperature
  int cvtTemp = this->tempCVT;
  String cvtString = "CVT: " + (String)cvtTemp + "C";
  char cvtChar[20];
  cvtString.toCharArray(cvtChar, 20);
  GD.cmd_text(10, 70, 31, OPT_CENTERY, cvtChar);

  // Display ambient temperature
  int ambTemp = this->tempAmb;
  String ambString = "Ambient: " + (String)ambTemp + "C / " + (String)((int)(1.8*ambTemp+32)) + "F";
  char ambChar[20];
  ambString.toCharArray(ambChar, 20);
  GD.cmd_text(10, 30, 31, OPT_CENTERY, ambChar);

  // Display engine RPM
  int deleteThis = this->rpm;
  deleteThis = deleteThis + rand()%25 - rand()%25;
  String rpmString = (String)deleteThis;
  char rpmChar[10];
  rpmString.toCharArray(rpmChar, 10);
  GD.cmd_gauge(170, GD.h/2+50, 200, OPT_NOBACK, 20, 100, deleteThis, 750);
  GD.cmd_text(170, GD.h/2+100, 31, OPT_CENTER, rpmChar);
  GD.cmd_text(170, GD.h/2+130, 29, OPT_CENTER, "RPM");

  // Display vehicle speed
  String speedString = (String)this->speedMPH;
  char speedChar[10];
  speedString.toCharArray(speedChar, 10);
  GD.cmd_gauge(GD.w-170, GD.h/2+50, 200, OPT_NOBACK, 5, 10, this->speedMPH, 40);
  GD.cmd_text(GD.w-170, GD.h/2+100, 31, OPT_CENTER, speedChar);
  GD.cmd_text(GD.w-170, GD.h/2+130, 29, OPT_CENTER, "MPH");

  //Display fuel level
  String fuelString = (String)this->fuel;
  char fuelChar[10];
  fuelString.toCharArray(fuelChar, 10);
  GD.cmd_text(75, GD.h-25, 31, OPT_CENTER, "F:        %");
  GD.cmd_text(75, GD.h-25, 31, OPT_CENTER, fuelChar);

  if(fuel > 49)
    GD.ColorRGB(0, 255, 0); // Green
  else if(fuel > 20)
    GD.ColorRGB(255, 255, 0); // Yellow
  else
    GD.ColorRGB(255, 0, 0); // Red

  GD.cmd_progress(160, GD.h-40, GD.w-180, 30, OPT_CENTER, this->fuel, 100);

  this->fuel = this->fuel-1;
  if(this->fuel < 0){ // REMOVE THIS WHEN ACTUAL FUEL PUT IN
    this->fuel = 100;
  }
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
