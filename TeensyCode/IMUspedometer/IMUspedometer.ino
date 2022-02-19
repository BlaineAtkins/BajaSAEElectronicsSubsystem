#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

double velocityX=0;
double velocityY=0;
double angle=0;
double lastAccelValY=0;
double lastAccelValX=0;
double lastAngVelocityZ=0;

unsigned long imuSpeedTimer=0;

void setup() {
  Serial.begin(9600);
  /* Initialise BNO055 IMU */
  if(!bno.begin()){
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  }
  //delay(1000); //maybe necessary to give BNO055 time to initialize??
  bno.setExtCrystalUse(true);

  imuSpeedTimer=millis(); //called at END of setup
}

void loop() {
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  //Serial.print("X: ");Serial.print(gyro.x());Serial.print(" Y: ");Serial.print(gyro.y());Serial.print(" Z: ");Serial.print(gyro.z());Serial.println("");

  //get X speed
  double newSliceArea = 0.5*(millis()-imuSpeedTimer)/1000*(lastAccelValX+accel.x()); //divide by 1000 for millis??
  velocityX+=newSliceArea;
  lastAccelValX=accel.x();

  newSliceArea = 0.5*(millis()-imuSpeedTimer)/1000*(lastAccelValY+accel.y()); //divide by 1000 for millis??
  velocityY+=newSliceArea;
  lastAccelValY=accel.y();
  imuSpeedTimer=millis();

  newSliceArea = 0.5*(millis()-imuSpeedTimer)/1000*(lastAngVelocityZ+gyro.z()); //divide by 1000 for millis??
  angle+=newSliceArea;
  lastAngVelocityZ=gyro.z();
  
  Serial.print("SpeedX: ");Serial.print(velocityX);Serial.print("  SpeedY: ");Serial.print(velocityY);Serial.print("  Angle (deg): ");Serial.print(angle);Serial.println();
  
}
