#include <EEPROM.h> // For display
#include <SPI.h>
#include <GD2.h> // For display

// NOTE: Display uses following pins:
// 6-13 ()

void setup() {
  // Initialize Pins
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  // Initizlize Communications
  Serial.begin(9600);
  GD.begin(0);
}

void loop() {
  // put your main code here, to run repeatedly:
  Display();
  delay(100);
}

void Display(){
  // Collect/Retrieve data
  int tempAmb = GetTempAmb();
  int tempCVT = GetTempCVT();
  int rpm = GetRPM();
  int fuelLevel = GetFuelLevel();
  
  // Formatting
  GD.ClearColorRGB(0, 0, 40); // Set background color
  GD.Clear();
  GD.ColorRGB(0xffffff); // Set text/element color


  // Display engine RPM
  String rpmString = (String)rpm;
  char rpmChar[10];
  rpmString.toCharArray(rpmChar, 10);
  GD.cmd_gauge(GD.w/2, GD.h/2, 100, OPT_NOBACK, 20, 100, rpmChar, 750);
  GD.cmd_text(GD.w/2, GD.h/2+50, 31, OPT_CENTER, rpmChar);
  GD.cmd_text(GD.w/2, GD.h/2+80, 29, OPT_CENTER, "RPM");

  //Display fuel level
  GD.cmd_progress(GD.w-35, 70, 25, GD.h-90, OPT_CENTER, fuelLevel, 100);
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

// TODO: Get ambient temperature
int GetTempAmb(){
  return 70;
}

// TODO: Get ambient temperature
int GetTempCVT(){
  return 70;
}

// TODO: Get engine rpm
int GetRPM(){
  return 500;
}

// TODO: Get fuel level (0-100%)
int GetFuelLevel(){
  return 85;
}
