#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7,8); //CE, CSN
//MAKE SURE YOU CHANGE DIV2 IN RF24.cpp TO DIV4!!!

const byte address[6] = "00011";

void setup() {
  Serial.begin(9600);
  Serial.println(radio.begin());
  delay(1000);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  radio.setAutoAck(false);

  radio.setDataRate(RF24_250KBPS);

  pinMode(2,OUTPUT);
}

void loop() {
  if (radio.available()){
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    if(text[0]=='o' & text[1]=='n'){
      digitalWrite(2,HIGH);
    }else if(text[0]=='o' && text[1]=='f' && text[2]=='f'){
      digitalWrite(2,LOW);
    }
    
  }

}
