#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>
#include <SD.h>
#include <TinyGPS++.h> //DIFFERENT FROM TinyGPS! Forgot if this is from arduino library manager or github
#include "Vehicle.h"
#include <RF24.h>


//for ambient temp
#include <OneWire.h> //download from https://github.com/PaulStoffregen/OneWire for teensy compatible
#include <DallasTemperature.h> //download on step 3 from https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
#define ONE_WIRE_BUS 31
OneWire oneWire(ONE_WIRE_BUS); //this bus can be used for other OneWire devices too
DallasTemperature sensors(&oneWire);

//for GPS
TinyGPSPlus gps;

//for radio comms
RF24 radio(38,0); //ce, csn -- MAKE SURE TO CHANGE SPI TO SPI1 in library /utility/Teensy/RF24_arch_config.h
byte address[6]="2Node";
bool radioDetected=0;

// Initialize display (and other things)
void Vehicle::BeginDisplay(){
  Serial.println("If you get stuck here, the display is disconnected. Please connect display to boot.");
  SD.begin(254);  
  GD.begin(0);
  //for ambient temperature
  sensors.begin();
  sensors.setWaitForConversion(false); //don't wait ~750 ms for temp to return new value

  //Read in Fuel Level from last line of SD card this->fuel
  String lastLines;
  if(SD.exists("VehicleData.csv")){
    File dataFile = SD.open("VehicleData.csv");
    //File dataFile = SD.open("test.txt");
    dataFile.seek(dataFile.size()-400); //go to near end of file so we don't read the entire file. Should subtract at least more than # of chars per line
    while(dataFile.available()){
      char c = dataFile.read();
      if (isPrintable(c) || c=='\n'){
        lastLines.concat(c);
      }
    }
    String newLine = lastLines.substring(lastLines.indexOf('\n')); //note that this does not necessarily return the LAST line, just near to the last line. This doesn't matter sice lines are added every few dozen milliseconds, during which fuel level will not have changed appriciably
    //Serial.println(newLine);
    int startIndex;
    int endIndex;
    int currIndex=0;
    for(int i=0;i<5;i++){
      currIndex=newLine.indexOf(',',currIndex)+1;
      if(i==3){
        startIndex=currIndex;
      }
      if(i==4){
        endIndex=currIndex-1;
      }
    }
    //Serial.println(newLine.substring(startIndex,endIndex));
    this->fuel = newLine.substring(startIndex,endIndex).toInt();
  }else{
    this->fuel=100;
  }

  //for radio
  if(radio.begin()){
    Serial.println("NRF24 radio present");
    radioDetected=1;
  }else{
    Serial.println("ERROR: Failed to detect NRF24 radio");
  }
  radio.openWritingPipe(address);
  radio.stopListening(); //don't listen for anything, we are only sending
  radio.setAutoAck(false); //just send, don't worry about success
  radio.setDataRate(RF24_250KBPS); //lowest data rate to maximize range
  
}

void Vehicle::WriteToRadio(){
  if(radioDetected){ //don't waste time timing out if we already know the radio isn't connected
    char buf[33];
    char sendText[33];
    //itoa(timePerCycle,buf,10);
    sprintf(buf,"a"); //pretext: 1 char
    strcpy(sendText,buf);
    sprintf(buf,"%.7d",millis()/1000); //runtime: 7 chars
    strcat(sendText,buf);

    //GPS date, 10 chars
    sprintf(buf,"%.2d/%.2d/%.4d",gpsMonth,gpsDay,gpsYear);
    strcat(sendText,buf);

    //GPS time, 8 chars
    sprintf(buf,"%.2d:%.2d:%.2d",gpsHour,gpsMinute,gpsSecond);
    strcat(sendText,buf);

    //code loop time, 4 chars (because on boot the first loop can take multiple seconds)
    sprintf(buf,"%.4d",timePerCycle);
    strcat(sendText,buf);

    //FIRST CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));


    sprintf(buf,"b"); //pretext: 1 char
    strcpy(sendText,buf);

    //Fuel level, 3 chars
    sprintf(buf,"%.3d",this->fuel);
    strcat(sendText,buf);

    //RPM, 4 chars
    sprintf(buf,"%.4d",this->rpm);
    strcat(sendText,buf);

    //speed from driveshaft, 5 chars
    dtostrf(speedMPH,5,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //speed from GPS, 5 chars
    dtostrf(gpsSpeed,5,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //gps heading, 6 chars
    dtostrf((this->gpsCourse/100.0),6,2,buf);
    strcat(sendText,buf);

    //SECOND CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"c"); //pretext: 1 char
    strcpy(sendText,buf);
    
    //latitude
    sprintf(buf,"%lf",this->gpsLat);
    strcat(sendText,buf);

    sprintf(buf,",");
    strcat(sendText,buf);
    //longitude
    sprintf(buf,"%lf",this->gpsLng);
    strcat(sendText,buf);

    //THIRD CHUNK DONE BECAUSE LAT/LNG ARE VARIABLE LENGTHS
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"d"); //pretext: 1 char
    strcpy(sendText,buf);

    //ambient temp, 7 chars (when floating, value can be in the negative hundreds, and that shouldn't break it)
    dtostrf(gpsSpeed,7,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //cvt belt temp, 3 chars
    sprintf(buf,"%.3d",this->tempCVT);
    strcat(sendText,buf);

    //electronics box temp, 2 chars   (this breaks deliniation when negative. ex, -4 becomes -04)
    sprintf(buf,"%.2d",this->boxTemp);
    strcat(sendText,buf);

    //raw x accel, 6 chars
    dtostrf(rawAccelx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //raw y accel, 6 chars
    dtostrf(rawAccely,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //raw z accel, 6 chars
    dtostrf(rawAccelz,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);
    
    //FOURTH CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"e"); //pretext: 1 char
    strcpy(sendText,buf);

    //x accel minus g, 6 chars
    dtostrf(linAccelx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //y accel minus g, 6 chars
    dtostrf(linAccelx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //z accel minus g, 6 chars
    dtostrf(linAccelx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //x gravity vector, 6 chars
    dtostrf(gravityx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //y gravity vector, 6 chars
    dtostrf(gravityy,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //FIFTH CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"f"); //pretext: 1 char
    strcpy(sendText,buf);

    //z gravity vector, 6 chars
    dtostrf(gravityz,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //x orientation, 6 chars
    dtostrf(orientationx,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //y orientation, 6 chars
    dtostrf(orientationy,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //z orientation, 6 chars
    dtostrf(orientationz,6,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);


    //SIXTH CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"g"); //pretext: 1 char
    strcpy(sendText,buf);

    //magnetic x, 8 chars (max sensor value is -2048.00 uT, which is possible if it is next to a magnet)
    dtostrf(magx,8,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //magnetic y, 8 chars (max sensor value is -2048.00 uT, which is possible if it is next to a magnet)
    dtostrf(magy,8,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //magnetic z, 8 chars (max sensor value is -2048.00 uT, which is possible if it is next to a magnet)
    dtostrf(magz,8,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //gyro x, 7 chars (can conciveably be in the negative hundreds
    dtostrf(gyrox,7,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);


    //SEVENTH CHUNK FULL
    radio.write(&sendText,strlen(sendText));
    //reset char arrays
    memset(&buf[0],0,sizeof(buf));
    memset(&sendText[0],0,sizeof(sendText));

    sprintf(buf,"h"); //pretext: 1 char
    strcpy(sendText,buf);

    //gyro y, 7 chars (can conciveably be in the negative hundreds
    dtostrf(gyroy,7,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);

    //gyro z, 7 chars (can conciveably be in the negative hundreds
    dtostrf(gyroz,7,2,buf); //value, min field width, # decimals, storage
    strcat(sendText,buf);
    
    radio.write(&sendText,strlen(sendText));
   
  }
}

int Vehicle::GetCycleTime(){
  this->timePerCycle = millis()-(this->cycleTimer);
  (this->cycleTimer)=millis();
  return(this->timePerCycle);
}

int Vehicle::GetSpeedMPH(){
  return this->speedMPH;
}


// TODO: include all data in write pass
// Write all data to SD card with time stamp
bool Vehicle::WriteToSD(){

  //if dataFile doesn't exist, write the header
  if(!SD.exists("VehicleData.csv")){
    File dataFile = SD.open("VehicleData.csv", FILE_WRITE);
    dataFile.print("Runtime (s),Date (GPS),Time (GPS),Code loop time (ms),Fuel level (%),Engine RPM,Speed (MPH from driveshaft),Speed (MPH from GPS),Heading (deg from GPS),Latitude,Longitude,Ambient Temp (deg C),CVT belt temp (deg C),Electronics Box temp (deg C),X Acceleration (m/s^2),y Acceleration (m/s^2),z Acceleration (m/s^2),X Acceleration minus g (m/s^2),y Acceleration minus g (m/s^2),Z Acceleration minus g (m/s^2),X gravity vector (m/s^2),Y gravity vector (m/s^2),Z gravity vector (m/s^2),X Orientation (degrees),Y Orientation (degrees),Z Orientation (degrees),X magnetic field (uT),Y magnetic field (uT),Z magnetic field (uT),X angular velocity (m/s),Y angular velocity (m/s),Z angular velocity (m/s)");
    dataFile.print("\n");
    dataFile.close();
  }
  
  File dataFile = SD.open("VehicleData.csv", FILE_WRITE);
  if(dataFile){ // 254==BUILTIN_SDCARD, set 
    dataFile.print(millis()/1000.0); //WARNING: if you modify this, pleaes ensure fuel remains the 5th column, or modify the code that gets fuel level on startup
    dataFile.print(",");
    dataFile.print(gpsMonth);
    dataFile.print("/");
    dataFile.print(gpsDay);
    dataFile.print("/");
    dataFile.print(gpsYear);
    dataFile.print(",");
    dataFile.print(gpsHour);
    dataFile.print(":");
    dataFile.print(gpsMinute);
    dataFile.print(":");
    dataFile.print(gpsSecond);
    dataFile.print(",");
    dataFile.print(timePerCycle);
    dataFile.print(",");
    dataFile.print(this->fuel); //WARNING: if you modify this, pleaes ensure fuel remains the 5th column, or modify the code that gets fuel level on startup
    dataFile.print(",");
    dataFile.print(this->rpm);
    dataFile.print(",");
    dataFile.print(this->speedMPH);
    dataFile.print(",");
    dataFile.print(this->gpsSpeed);
    dataFile.print(",");
    dataFile.print(this->gpsCourse);
    dataFile.print(",");
    dataFile.print(this->gpsLat);
    dataFile.print(",");
    dataFile.print(this->gpsLng);
    dataFile.print(",");
    dataFile.print(this->tempAmb);
    dataFile.print(",");
    dataFile.print(this->tempCVT);
    dataFile.print(",");
    dataFile.print(this->boxTemp);
    dataFile.print(",");

    //BELOW IS ALL FROM 9DOF

    dataFile.print(this->rawAccelx);
    dataFile.print(",");
    dataFile.print(this->rawAccely);
    dataFile.print(",");
    dataFile.print(this->rawAccelz);
    dataFile.print(",");
    dataFile.print(this->linAccelx);
    dataFile.print(",");
    dataFile.print(this->linAccely);
    dataFile.print(",");
    dataFile.print(this->linAccelz);
    dataFile.print(",");
    dataFile.print(this->gravityx);
    dataFile.print(",");
    dataFile.print(this->gravityy);
    dataFile.print(",");
    dataFile.print(this->gravityz);
    dataFile.print(",");
    dataFile.print(this->orientationx);
    dataFile.print(",");
    dataFile.print(this->orientationy);
    dataFile.print(",");
    dataFile.print(this->orientationz);
    dataFile.print(",");
    dataFile.print(this->magx);
    dataFile.print(",");
    dataFile.print(this->magy);
    dataFile.print(",");
    dataFile.print(this->magz);
    dataFile.print(",");
    dataFile.print(this->gyrox);
    dataFile.print(",");
    dataFile.print(this->gyroy);
    dataFile.print(",");
    dataFile.print(this->gyroz);
    
    dataFile.print("\n"); 
    
    dataFile.close();
    return true;    
  }
  dataFile.close();
  return false;
}

void Vehicle::DisplayGPSOnSerial(){ 
  Serial.print(F("Location: ")); 
  if (this->gpsLat!=NULL && this->gpsLng!=NULL){
    Serial.print(this->gpsLat, 6);
    Serial.print(F(","));
    Serial.print(this->gpsLng, 6);
  }
  else{
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (this->gpsMonth!=NULL && this->gpsDay!=NULL && this->gpsYear!=NULL){
    Serial.print(this->gpsMonth);
    Serial.print(F("/"));
    Serial.print(this->gpsDay);
    Serial.print(F("/"));
    Serial.print(this->gpsYear);
  }
  else{
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
//  if (this->gpsHour!=NULL && this->gpsMinute!=NULL && this->gpsSecond!=NULL){ //returns invalid when h/m/s are 0
    if (this->gpsHour < 10) Serial.print(F("0"));
    Serial.print(this->gpsHour);
    Serial.print(F(":"));
    if (this->gpsMinute < 10) Serial.print(F("0"));
    Serial.print(this->gpsMinute);
    Serial.print(F(":"));
    if (this->gpsSecond < 10) Serial.print(F("0"));
    Serial.print(this->gpsSecond);
/*  }
  else{
    Serial.print(F("INVALID"));
  }*/

  Serial.print("   Speed: ");
  if(this->gpsSpeed!=NULL){  
    Serial.print(this->gpsSpeed);
    Serial.print(" mph");
  }else{
    Serial.print("INVALID");
  }

  Serial.print("   Heading: ");
  if(this->gpsCourse!=NULL){
    Serial.print(this->gpsCourse/100.0);
    Serial.print(" degrees");
  }else{
    Serial.print("INVALID");
  }

  Serial.println();
}

void Vehicle::GetGPSData(){ //WARNING: if you only call this once per second, if you leave it running, within a minute you will notice that every once in awhile, if you print the data it gets stuck on one data point for several seconds. NOt sure why...
  //if (Serial5.available()>0 && gps.encode(Serial5.read())){ //if we're getting GPS data
  while(Serial5.available()){
    gps.encode(Serial5.read());
  }
 
    if (gps.location.isValid()){
      this->gpsLat=gps.location.lat();
      this->gpsLng=gps.location.lng();
    }else{
     this->gpsLat=NULL;
     this->gpsLng=NULL;
    }
  
    if (gps.date.isValid()){
      this->gpsMonth=gps.date.month();
      this->gpsDay=gps.date.day();
      this->gpsYear=gps.date.year();
    }else{
      this->gpsMonth=NULL;
      this->gpsDay=NULL;
      this->gpsYear=NULL;
    }
  
    if (gps.time.isValid()){
      this->gpsHour=gps.time.hour();
      this->gpsMinute=gps.time.minute();
      this->gpsSecond=gps.time.second();
    }
    else{
      this->gpsHour=NULL;
      this->gpsMinute=NULL;
      this->gpsSecond=NULL;
    }
    
    if(gps.speed.isValid()){
      this->gpsSpeed=gps.speed.mph();
    }else{
      this->gpsSpeed=NULL;
    }
    if(gps.course.isValid()){
      this->gpsCourse=gps.course.value();
    }else{
      this->gpsCourse=NULL;
    }
    //Serial.println(gps.time.second());
    //Serial.print("Age of GPS data: ");
    //Serial.println(gps.speed.age()); //not sure where/if this is useful yet...    
}


void Vehicle::Get9dofData(){
  //IF ACCELERATION VALUES ARE RETURNING INF, SET THE SENSITIVITY LOWER, THERE SHOULD BE A FUNCTION TO DO THAT
  //raw accelerometer (m/s/s)
  imu::Vector<3> rawAccel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  this->rawAccelx=rawAccel.x();
  this->rawAccely=rawAccel.y();
  this->rawAccelz=rawAccel.z();
 
  //Acceleration minus gravity (m/s/s)
  imu::Vector<3> linAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  this->linAccelx=linAccel.x();
  this->linAccely=linAccel.y();
  this->linAccelz=linAccel.z();

  //gravity vector (m/s/s)
  imu::Vector<3> gravity = bno.getVector(Adafruit_BNO055::VECTOR_GRAVITY);
  this->gravityx=gravity.x();
  this->gravityy=gravity.y();
  this->gravityz=gravity.z();

  //Orientation (degrees based on 360 degree sphere)
  sensors_event_t event; 
  bno.getEvent(&event);
  this->orientationx=event.orientation.x;
  this->orientationy=event.orientation.y;
  this->orientationz=event.orientation.z;

  //magnetometer (microtesla)
  imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
  this->magx=mag.x();
  this->magy=mag.y();
  this->magz=mag.z();

  //gyroscope (rad/s)
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  this->gyrox=gyro.x();
  this->gyroy=gyro.y();
  this->gyroz=gyro.z();

  //temp
  boxTemp = bno.getTemp();
}


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
  //Serial.println(this->tempCVT);
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
  speedString.remove(speedString.length()-1);
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
