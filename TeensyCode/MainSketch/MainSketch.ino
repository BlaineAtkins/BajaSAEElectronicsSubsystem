//#include <TinyGPS++.h> //DIFFERENT FROM TinyGPS! Forgot if this is from arduino library manager or github
//#include <SoftwareSerial.h> //also forget if this is from arduino library manager or github for 9dof
#include <Wire.h>
#include <SPI.h>
//for BNO055
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
//#include <Adafruit_LSM9DS1.h> //installed from Arduino library manager, search for Adafruit LSM9DS1 and click yes to install dependancies
//#include <Adafruit_Sensor.h>  // not used but required!
#include "Vehicle.h"
// NOTE: Display uses pins 8-13

//for hall effect spedometer
int numMagnets=4;
float tireDiameter=0.6604;
float driveshaftRatio=2; //ex: if driveshaft spins twice as fast as tires, driveshaftRatio=2
unsigned long timerVehicleSpeed;
int runningAvgHallSpedometer[4];

float vehicleSpeedMPH=0;

void setup() {
  
  // Initizlize Communications
  Serial.begin(115200);

  Serial5.begin(9600); //initialize GPS hardware serial

  //Hall Effect spedometer
  pinMode(2,INPUT); //for hall effect MPH sensor -- must be an interrupt capable pin.
  attachInterrupt(digitalPinToInterrupt(2), InterruptMagSpeedTransition, CHANGE); //set ISR for hall effect sensor

  //CVT infrared sensor
  pinMode(41,INPUT);

  //BNO055 IMU 9dof
}
void loop() {
  Vehicle Baja;
  
/* GPS setup to be parsed by Blaine
  if (ss.available()>0 && gps.encode(ss.read())){ //if we're getting GPS data
    handleGPS();
    //if statement does not fire if we're doing too much outside of this... (ex, calling handle9dof() or display___onSerial() in the main loop). ...Is that because serial data from GPS needs to be received synchronously? Sounds like software serial is generally inferior to hardware serial. I think we can shift this to hardware serial with Teensy 4.1 see https://www.pjrc.com/teensy/td_uart.html
    displayGPSOnSerial();
  }
  //handle9dof();
  //display9dofOnSerial();
  displayToLCD();
  writeToSDCard();
  transmitToRadio();
*/

  while(1){
    Baja.Display();
    delay(10);
    if(millis()-timerVehicleSpeed>1000){
      vehicleSpeedMPH=0;
    }
    Baja.speedMPH=vehicleSpeedMPH; //vehicleSpeedMPH is a global variable set by InterruptMagSpeedTransition()

    Baja.GetTempAmb();
    Baja.GetTempCVT();
    
    Baja.GetGPSData(); //Call this as often as possible! See comment on the function in Vehicle.cpp
    //Baja.DisplayGPSOnSerial();

    Baja.Get9dofData();

    Baja.GetCycleTime();

    Baja.WriteToSD();
    
  }
  exit(0);
}

void InterruptMagSpeedTransition(){
  float rpm;
  float hz;
  float vehicleSpeedMetersPS;
  float timeBetweenUpdates;
  
  long elapsedTimeVehicleSpeed=millis()-timerVehicleSpeed;
  timerVehicleSpeed=millis(); //reset timerVehicleSpeed
  timeBetweenUpdates=elapsedTimeVehicleSpeed/1000.0;

  //Average last few values using HZ
  runningAvgHallSpedometer[0]=runningAvgHallSpedometer[1];
  runningAvgHallSpedometer[1]=runningAvgHallSpedometer[2];
  runningAvgHallSpedometer[2]=runningAvgHallSpedometer[3];
  runningAvgHallSpedometer[3]=elapsedTimeVehicleSpeed;

  elapsedTimeVehicleSpeed=0;
  for(int i=0;i<4;i++){
    elapsedTimeVehicleSpeed+=runningAvgHallSpedometer[i];
  }
  elapsedTimeVehicleSpeed=elapsedTimeVehicleSpeed/4;
  
  hz = 1.0/(((elapsedTimeVehicleSpeed/1000.0)*numMagnets)); 

  rpm = hz*60;
  Serial.println(rpm);

  float pointDistance = (PI*tireDiameter)/numMagnets; //distance between two adjacent magnets, scaled to distance on tire
  vehicleSpeedMetersPS = pointDistance/(elapsedTimeVehicleSpeed/1000.0); // m/s
  vehicleSpeedMPH = vehicleSpeedMetersPS*(1/1609.34)*(3600/1);      //*(miles/meter)*(seconds/hour)
  vehicleSpeedMetersPS=vehicleSpeedMetersPS/driveshaftRatio;
  vehicleSpeedMPH=vehicleSpeedMPH/driveshaftRatio;
}


/*
void displayToLCD(){
  bool validSpeed=true;
  if(gpsSpeed==NULL || gps.speed.age()>2000){ //if we didn't get a valid speed or the speed data was collected over 2 seconds ago
    validSpeed=false; //display -- or something instead of speed
  }
}



void transmitToRadio(){
  //test what happens when you attempt to transmit NULL
}



void display9dofOnSerial(){
  Serial.print("Accel X: "); Serial.print(ndofAccelX); Serial.print(" m/s^2");
  Serial.print("  Y: "); Serial.print(ndofAccelY);     Serial.print(" m/s^2 ");
  Serial.print("  Z: "); Serial.print(ndofAccelZ);     Serial.print(" m/s^2 ");
  Serial.print("\t");
  Serial.print("Mag X: "); Serial.print(ndofMagX);   Serial.print(" uT");
  Serial.print("  Y: "); Serial.print(ndofMagY);     Serial.print(" uT");
  Serial.print("  Z: "); Serial.print(ndofMagZ);     Serial.print(" uT");
  Serial.print("\t");
  Serial.print("Gyro X: "); Serial.print(ndofGyroX);   Serial.print(" rad/s");
  Serial.print("  Y: "); Serial.print(ndofGyroY);      Serial.print(" rad/s");
  Serial.print("  Z: "); Serial.print(ndofGyroZ);      Serial.print(" rad/s");

  Serial.println();
}







void handle9dof(){
  lsm.read();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp);

  ndofAccelX=a.acceleration.x; // m/s^2
  ndofAccelY=a.acceleration.y; // m/s^2
  ndofAccelZ=a.acceleration.z; // m/s^2
  ndofMagX=m.magnetic.x;   // uT
  ndofMagY=m.magnetic.y;   // uT
  ndofMagZ=m.magnetic.z;   // uT
  ndofGyroX=g.gyro.x;  // rad/s
  ndofGyroY=g.gyro.y;  // rad/s
  ndofGyroZ=g.gyro.z;  // rad/s
}



void handleGPS(){ 
  if (gps.location.isValid()){
    gpsLat=gps.location.lat();
    gpsLng=gps.location.lng();
  }else{
   gpsLat=NULL;
   gpsLng=NULL;
  }

  if (gps.date.isValid()){
    gpsMonth=gps.date.month();
    gpsDay=gps.date.day();
    gpsYear=gps.date.year();
  }else{
    gpsMonth=NULL;
    gpsDay=NULL;
    gpsYear=NULL;
  }

  if (gps.time.isValid()){
    gpsHour=gps.time.hour();
    gpsMinute=gps.time.minute();
    gpsSecond=gps.time.second();
  }
  else{
    gpsHour=NULL;
    gpsMinute=NULL;
    gpsSecond=NULL;
  }
  
  if(gps.speed.isValid()){
    gpsSpeed=gps.speed.mph();
  }else{
    gpsSpeed=NULL;
  }
  if(gps.course.isValid()){
    gpsCourse=gps.course.value();
  }else{
    gpsCourse=NULL;
  }
  //Serial.print(gps.speed.age()); //not sure where/if this is useful yet...
}
*/
