//#include <EEPROM.h> // For display
//#include <SPI.h>
//#include "Vehicle.cpp"


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
  Vehicle Baja;

  while(1){
    Baja.Display();
    delay(50);
  }
  exit(0);
}


