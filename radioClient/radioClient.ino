#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7,8); //CE, CSN

//MAKE SURE YOU CHANGE DIV2 IN RF24.cpp TO DIV4!!!

const byte address[6] = "2Node";

//A block
char runtime[8];
char gpsDate[11];
char gpsTime[9];
char timePerCycle[5];

//B block
char fuel[4];
char rpm[5];
char driveshaftSpeed[6];
char gpsSpeed[6];
char gpsHeading[7];

//C block
char lat[15];
char lng[15];

//D block
char tempAmbient[8];
char tempCVT[4];
char tempBox[3];
char rawAccelx[7];
char rawAccely[7];
char rawAccelz[7];

//E block
char linAccelx[7];
char linAccely[7];
char linAccelz[7];
char gravityx[7];
char gravityy[7];

//F block
char gravityz[7];
char orientationx[7];
char orientationy[7];
char orientationz[7];

//G block
char magx[9];
char magy[9];
char magz[9];
char gyrox[8];

//H block
char gyroy[8];
char gyroz[8];

unsigned long ageNewestData=0;
unsigned long ageOldestData=0;
unsigned long lastTimeDisplayedData=0;

const int numBlocks=8;
unsigned long lastTimeReceived[numBlocks];
unsigned long age[numBlocks];

//FOR PLOTTING GPS POSITION VS SIGNAL STRENGTH
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin=4, TXPin=3;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin,TXPin);
int samplesInLastPeriod=0; //currently, one period is ~2 seconds (every other GPS receipt)
int period=1000; //time period in which to sample received packets
unsigned long lastCountReset=0;


void setup() {
  Serial.begin(115200);
  if(radio.begin()){
    Serial.println("Radio initialized - ready to receive data...");
  }else{
    Serial.println("RADIO NOT DETECTED. Please check wiring.");
  }
  delay(1000);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);

  pinMode(2,OUTPUT);

  //FOR PLOTTING GPS POSITION VS SIGNAL STRENGTH
  ss.begin(GPSBaud);
}

void loop() {
  getData();
  getAgeOfData();
  if(ageNewestData<(millis()-lastTimeDisplayedData)){ //only print another line if we've received new data since then
    displayData();
  }
  statusLED();

  //printGPS();


}

void printGPS(){
  while (ss.available() > 0)
    gps.encode(ss.read());
    
    if(millis()-lastCountReset>period){
    Serial.print(gps.location.lat(), 6);
    Serial.print(",");
    Serial.print(gps.location.lng(), 6);
    Serial.print(",");
    Serial.println(samplesInLastPeriod);
    samplesInLastPeriod=0;
    lastCountReset=millis();
  }
}

void statusLED(){
  if(ageNewestData<40){
    digitalWrite(2,HIGH);
  }else{
    digitalWrite(2,LOW);
  }
    
}

void displayData(){
  printWithDeliniator(runtime);
  printWithDeliniator(gpsDate);
  printWithDeliniator(gpsTime);
  printWithDeliniator(timePerCycle);
  printWithDeliniator(fuel);
  printWithDeliniator(rpm);
  printWithDeliniator(driveshaftSpeed);
  printWithDeliniator(gpsSpeed);
  printWithDeliniator(gpsHeading);
  printWithDeliniator(lat);
  printWithDeliniator(lng);
  printWithDeliniator(tempAmbient);
  printWithDeliniator(tempCVT);
  printWithDeliniator(tempBox);
  printWithDeliniator(rawAccelx);
  printWithDeliniator(rawAccely);
  printWithDeliniator(rawAccelz);
  printWithDeliniator(linAccelx);
  printWithDeliniator(linAccely);
  printWithDeliniator(linAccelz);
  printWithDeliniator(gravityx);
  printWithDeliniator(gravityy);
  printWithDeliniator(gravityz);
  printWithDeliniator(orientationx);
  printWithDeliniator(orientationy);
  Serial.print(orientationz);

  if(ageOldestData>2000){ //notify user that some data is outdated
    Serial.print("\t!");
  }

  Serial.println();
  lastTimeDisplayedData=millis();
}

void printWithDeliniator(char *toPrint){
  char deliniator[5]="  ";
  Serial.print(toPrint);
  Serial.print(deliniator);
}

void getAgeOfData(){
  //update how old data is
  for(int i=0;i<numBlocks;i++){
    age[i]=millis()-lastTimeReceived[i];
  }

  ageOldestData = 0;
  for(int i=0;i<numBlocks;i++){
    if(age[i]>ageOldestData){
      ageOldestData=age[i];
    }
  }
  //Serial.println(ageOldestData);
  ageNewestData = age[0];
  for(int i=0;i<numBlocks;i++){
    if(age[i]<ageNewestData){
      ageNewestData=age[i];
    }
  }
  
}

void getData(){
  if (radio.available()){
    char text[32] = "";
    radio.read(&text, sizeof(text));
    //Serial.println(text);

    //FOR GPS LOGGING
    samplesInLastPeriod++;
    
    if(text[0]=='a'){
      lastTimeReceived[0]=millis();
      copySubstring(text,runtime,1,8);
      copySubstring(text,gpsDate,8,18);
      copySubstring(text,gpsTime,18,26);
      copySubstring(text,timePerCycle,26,30);
    }
        
    if(text[0]=='b'){
      lastTimeReceived[1]=millis();
      copySubstring(text,fuel,1,4);
      copySubstring(text,rpm,4,8);
      copySubstring(text,driveshaftSpeed,8,13);
      copySubstring(text,gpsSpeed,13,18);
      copySubstring(text,gpsHeading,18,24);             
    }
    if(text[0]=='c'){
      lastTimeReceived[2]=millis();
      int commaIndex=0;
      for(int i=0;i<sizeof(text);i++){
        if(text[i]==','){
          commaIndex=i;
        }
      }
      int stringEnd=0;
      for(int i=0;i<sizeof(text);i++){
        if(text[i]=='.'){
          stringEnd=i;
        }
      }
      stringEnd+=7; //this will always have 6 decimal places
      
      copySubstring(text,lat,1,commaIndex);
      copySubstring(text,lng,commaIndex+1,stringEnd);
    }
    if(text[0]=='d'){
      lastTimeReceived[3]=millis();
      copySubstring(text,tempAmbient,1,8);
      copySubstring(text,tempCVT,9,11);
      copySubstring(text,tempBox,11,13);
      copySubstring(text,rawAccelx,13,19);
      copySubstring(text,rawAccely,19,25);
      copySubstring(text,rawAccelz,25,31);
    }
    if(text[0]=='e'){
      lastTimeReceived[4]=millis();
      copySubstring(text,linAccelx,1,7);
      copySubstring(text,linAccely,7,13);
      copySubstring(text,linAccelz,13,19);
      copySubstring(text,gravityx,19,25);
      copySubstring(text,gravityy,25,31);
    }
    if(text[0]=='f'){
      lastTimeReceived[5]=millis();
      copySubstring(text,gravityz,1,7);
      copySubstring(text,orientationx,7,13);
      copySubstring(text,orientationy,13,19);
      copySubstring(text,orientationz,19,25);
    }
    if(text[0]=='g'){
      lastTimeReceived[6]=millis();
      copySubstring(text,magx,1,9);
      copySubstring(text,magy,9,17);
      copySubstring(text,magz,17,25);
      copySubstring(text,gyrox,25,32);
    }
    if(text[0]=='h'){
      lastTimeReceived[7]=millis();
      copySubstring(text,gyroy,1,8);
      copySubstring(text,gyroz,8,15);
    }
  }
}

void copySubstring(char *source, char *dest, int startIndex, int endIndex){
  memset(&dest[0],0,sizeof(dest)); //clear array before copying new values in
  for(int i=startIndex;i<endIndex;i++){
    dest[i-startIndex]=source[i];
  }
}
