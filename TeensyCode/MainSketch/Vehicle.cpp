#include "Vehicle.h"
//#include <SPI.h>
//#include <GD2.h> // For display
#include <time.h>

Vehicle::Vehicle(){
    this.startTime = time(0);
    this.runTime = 0;
}



int Vehicle::GetRunTime(){
    this.runTime = time(0) - this.startTime;
}



// TODO: Get ambient temperature
int Vehicle::GetTempAmb(){
  return this.tempAmb;
}



// TODO: Get ambient temperature
int Vehicle::GetTempCVT(){
  return this.tempCVT;
}



// TODO: Get engine rpm
int Vehicle::GetRPM(){
  return this.rpm;
}



// TODO: Get fuel level (0-100%)
int Vehicle::GetFuelLevel(){
  return this.fuel;
}



void Display(){
 
  // Formatting
  GD.ClearColorRGB(0, 0, 40); // Set background color
  GD.Clear();
  GD.ColorRGB(0xffffff); // Set text/element color

  // Display engine RPM
  String rpmString = (String)this.rpm;
  char rpmChar[10];
  rpmString.toCharArray(rpmChar, 10);
  GD.cmd_gauge(GD.w/2, GD.h/2, 100, OPT_NOBACK, 20, 100, rpmChar, 750);
  GD.cmd_text(GD.w/2, GD.h/2+50, 31, OPT_CENTER, rpmChar);
  GD.cmd_text(GD.w/2, GD.h/2+80, 29, OPT_CENTER, "RPM");

  //Display fuel level
  GD.cmd_progress(GD.w-35, 70, 25, GD.h-90, OPT_CENTER, this.fuel, 100);
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










