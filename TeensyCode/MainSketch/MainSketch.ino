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
float driveshaftRatio=3; //ex: if driveshaft spins twice as fast as tires, driveshaftRatio=2
volatile unsigned long timerVehicleSpeed;
volatile int runningAvgHallSpedometer[4];
volatile float vehicleSpeedMPH=0;

//for induction pickup RPM
volatile unsigned long timerSparkPlug;
volatile int runningAvgSparkPlug[4];
volatile float engineRPM=0;


void setup() {
  
  // Initizlize Communications
  Serial.begin(115200);

  Serial5.begin(9600); //initialize GPS hardware serial

  //Hall Effect spedometer
  pinMode(2,INPUT); //for hall effect MPH sensor -- must be an interrupt capable pin.
  attachInterrupt(digitalPinToInterrupt(2), InterruptMagSpeedTransition, CHANGE); //set ISR for hall effect sensor

  //Induction Pickup RPM
  pinMode(7,INPUT);
  attachInterrupt(digitalPinToInterrupt(7), InterruptSparkPlugFire, RISING);

  //CVT infrared sensor
  pinMode(41,INPUT);

}
void loop() {
  Vehicle Baja;
  
  while(1){
    
    Baja.Display();
    delay(10);
    if(millis()-timerVehicleSpeed>1000){
      vehicleSpeedMPH=0;
    }
    if(millis()-timerSparkPlug>100){ //we expect spark plug fires a maximum of 40 ms apart
      engineRPM=0;
    }
    
    Baja.speedMPH=vehicleSpeedMPH; //vehicleSpeedMPH is a global variable set by InterruptMagSpeedTransition()
    Baja.rpm=engineRPM;
    
    Baja.GetTempAmb();
    Baja.GetTempCVT();
    
    Baja.GetGPSData(); //Call this as often as possible! See comment on the function in Vehicle.cpp
    //Baja.DisplayGPSOnSerial();

    Baja.Get9dofData();

    Baja.GetCycleTime();

    Baja.WriteToSD();
    Baja.WriteToRadio(); 

    
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
  //Serial.println(rpm);

  float pointDistance = (PI*tireDiameter)/numMagnets; //distance between two adjacent magnets, scaled to distance on tire
  vehicleSpeedMetersPS = pointDistance/(elapsedTimeVehicleSpeed/1000.0); // m/s
  vehicleSpeedMPH = vehicleSpeedMetersPS*(1/1609.34)*(3600/1);      //*(miles/meter)*(seconds/hour)
  vehicleSpeedMetersPS=vehicleSpeedMetersPS/driveshaftRatio;
  vehicleSpeedMPH=vehicleSpeedMPH/driveshaftRatio;
  //Serial.println(vehicleSpeedMPH);
}


void InterruptSparkPlugFire(){
  float hz;
  float timeBetweenUpdates;
  
  long elapsedTimeSparkPlug=millis()-timerSparkPlug;
  timerSparkPlug=millis(); //reset timerSparkPlug
  timeBetweenUpdates=elapsedTimeSparkPlug/1000.0;

  //Average last few values using HZ
  runningAvgSparkPlug[0]=runningAvgSparkPlug[1];
  runningAvgSparkPlug[1]=runningAvgSparkPlug[2];
  runningAvgSparkPlug[2]=runningAvgSparkPlug[3];
  runningAvgSparkPlug[3]=elapsedTimeSparkPlug;

  elapsedTimeSparkPlug=0;
  for(int i=0;i<4;i++){
    elapsedTimeSparkPlug+=runningAvgSparkPlug[i];
  }
  elapsedTimeSparkPlug=elapsedTimeSparkPlug/4;
  
  hz = 1.0/(((elapsedTimeSparkPlug/1000.0))); 

  engineRPM = hz*60;

}
