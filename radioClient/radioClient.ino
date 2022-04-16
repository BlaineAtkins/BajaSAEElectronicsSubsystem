#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7,8); //CE, CSN

//MAKE SURE YOU CHANGE DIV2 IN RF24.cpp TO DIV4!!!

const byte address[6] = "2Node";

unsigned long lastTimeReceivedA=0;
unsigned long ageA=0;
char runtime[8];
char gpsDate[11];
char gpsTime[9];
char timePerCycle[5];

unsigned long lastTimeReceivedB=0;
unsigned long ageB=0;
char fuel[4];
char rpm[5];
char driveshaftSpeed[6];
char gpsSpeed[6];
char gpsHeading[7];

unsigned long lastTimeReceivedC=0;
unsigned long ageC=0;
char lat[15];
char lng[15];

unsigned long lastTimeReceivedD=0;
unsigned long ageD=0;
char tempAmbient[8];
char tempCVT[4];
char tempBox[3];
char rawAccelx[7];
char rawAccely[7];
char rawAccelz[7];

unsigned long lastTimeReceivedE=0;
unsigned long ageE;
char linAccelx[7];
char linAccely[7];
char linAccelz[7];
char gravityx[7];
char gravityy[7];

void setup() {
  Serial.begin(115200);
  Serial.println(radio.begin());
  delay(1000);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  radio.setAutoAck(false);

  radio.setDataRate(RF24_250KBPS);
}

void loop() {
  //update how old data is
  ageA=millis()-lastTimeReceivedA;
  ageB=millis()-lastTimeReceivedB;
  ageC=millis()-lastTimeReceivedC;
  ageD=millis()-lastTimeReceivedD;
  ageE=millis()-lastTimeReceivedE;
  
  if (radio.available()){
    char text[32] = "";
    radio.read(&text, sizeof(text));
    //Serial.println(text);
    
    if(text[0]=='a'){
      lastTimeReceivedA=millis();
      copySubstring(text,runtime,1,8);
      copySubstring(text,gpsDate,8,18);
      copySubstring(text,gpsTime,18,26);
      copySubstring(text,timePerCycle,26,30);
    }
        
    if(text[0]=='b'){
      lastTimeReceivedB=millis();
      copySubstring(text,fuel,1,4);
      copySubstring(text,rpm,4,8);
      copySubstring(text,driveshaftSpeed,8,13);
      copySubstring(text,gpsSpeed,13,18);
      copySubstring(text,gpsHeading,18,24);             
    }
    if(text[0]=='c'){
      lastTimeReceivedC=millis();
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
      lastTimeReceivedD=millis();
      copySubstring(text,tempAmbient,1,8);
      copySubstring(text,tempCVT,9,11);
      copySubstring(text,tempBox,11,13);
      copySubstring(text,rawAccelx,13,19);
      copySubstring(text,rawAccely,19,25);
      copySubstring(text,rawAccelz,25,31);
    }
    if(text[0]=='e'){
      lastTimeReceivedE=millis();
    }
    
  }
}

void copySubstring(char *source, char *dest, int startIndex, int endIndex){
  memset(&dest[0],0,sizeof(dest)); //clear array before copying new values in
  for(int i=startIndex;i<endIndex;i++){
    dest[i-startIndex]=source[i];
  }
}
