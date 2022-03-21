//#include <TinyGPS++.h> //DIFFERENT FROM TinyGPS! Forgot if this is from arduino library manager or github
//#include <SoftwareSerial.h> //also forget if this is from arduino library manager or github for 9dof
#include <Wire.h>
#include <SPI.h>
//#include <Adafruit_LSM9DS1.h> //installed from Arduino library manager, search for Adafruit LSM9DS1 and click yes to install dependancies
//#include <Adafruit_Sensor.h>  // not used but required!
#include "Vehicle.h"
// NOTE: Display uses pins 8-13



// TODO: Do these need to be global? My computer science teacher will inflict
//       psychic damage to me from across campus if I use global variables
//       when they could be local variables in a function (or a class)
//Global Variables    set to NULL for invalid
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

double ndofAccelX; // m/s^2
double ndofAccelY; // m/s^2
double ndofAccelZ; // m/s^2
double ndofMagX;   // uT
double ndofMagY;   // uT
double ndofMagZ;   // uT
double ndofGyroX;  // rad/s
double ndofGyroY;  // rad/s
double ndofGyroZ;  // rad/s

//for hall effect spedometer
int numMagnets=4;
float tireDiameter=0.6604;
unsigned long timerVehicleSpeed;
int runningAvgHallSpedometer[4];


float vehicleSpeedMPH=0;


// TODO: This probably shouldn't be global
//for GPS
static const int gpsRXPin = 4, gpsTXPin = 3;
static const uint32_t GPSBaud = 9600; 
//TinyGPSPlus gps; // The TinyGPS++ object
//SoftwareSerial ss(gpsRXPin, gpsTXPin); // The serial connection to the GPS device

// TODO: defines can go at the top, lsm probably shouldn't be global
//for 9dof
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
//#define LSM9DS1_SCK A5
//#define LSM9DS1_MOSI A4


void setup() {
  
  // Initizlize Communications
  Serial.begin(115200);

  //Hall Effect spedometer
  pinMode(2,INPUT); //for hall effect MPH sensor -- must be an interrupt capable pin.
  attachInterrupt(digitalPinToInterrupt(2), InterruptMagSpeedTransition, CHANGE); //set ISR for hall effect sensor

/*
  ss.begin(GPSBaud);

  //9dof setup (accelerometer, magnetometer, gyroscope)
  if (!lsm.begin()){ //if 9dof sensor is detected
    Serial.println("Problem initializing LSM9DS1 9dof -- check wiring connections");
  }
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G); //Accelerometer range can be set range higher by changing 2G to 4G, 8G or 16G
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS); //magnetometer sensitivity can be set higher with 8GAUSS 12GAUSS or 16GAUSS
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS); //Gyroscope can be changed to 500DPS or 2000DPS
*/
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
    delay(50);

    if(millis()-timerVehicleSpeed>1000){
      vehicleSpeedMPH=0;
    }
    Baja.speedMPH=vehicleSpeedMPH; //vehicleSpeedMPH is a global variable set by InterruptMagSpeedTransition()

    Baja.GetTempAmb();
    
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



void displayGPSOnSerial(){ //ONLY WORKS IF CALLED IN GPS READ IF STATEMENT. Likely because communicating with GPS receiver over serial conflicts with printing this much data to serial monitor. Teensy 4.1 has 8 serial ports instead of Nano's one, so we should be able to solve this by using a seperate serial port
  Serial.print(F("Location: ")); 
  if (gpsLat!=NULL && gpsLng!=NULL){
    Serial.print(gpsLat, 6);
    Serial.print(F(","));
    Serial.print(gpsLng, 6);
  }
  else{
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gpsMonth!=NULL && gpsDay!=NULL && gpsYear!=NULL){
    Serial.print(gpsMonth);
    Serial.print(F("/"));
    Serial.print(gpsDay);
    Serial.print(F("/"));
    Serial.print(gpsYear);
  }
  else{
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gpsHour!=NULL && gpsMinute!=NULL && gpsSecond!=NULL){
    if (gpsHour < 10) Serial.print(F("0"));
    Serial.print(gpsHour);
    Serial.print(F(":"));
    if (gpsMinute < 10) Serial.print(F("0"));
    Serial.print(gpsMinute);
    Serial.print(F(":"));
    if (gpsSecond < 10) Serial.print(F("0"));
    Serial.print(gpsSecond);
  }
  else{
    Serial.print(F("INVALID"));
  }

  Serial.print("   Speed: ");
  if(gpsSpeed!=NULL){  
    Serial.print(gpsSpeed);
    Serial.print(" mph");
  }else{
    Serial.print("INVALID");
  }

  Serial.print("   Heading: ");
  if(gpsCourse!=NULL){
    Serial.print(gpsCourse/100.0);
    Serial.print(" degrees");
  }else{
    Serial.print("INVALID");
  }

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
