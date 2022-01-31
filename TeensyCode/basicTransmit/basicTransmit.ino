#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

RF24 radio(7,8); //(CE, CSN)

const byte address[6] = "00011";

unsigned long timer=0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  radio.setAutoAck(false);

  radio.setDataRate(RF24_250KBPS);
}

void loop() {

      timer=micros();
      
      const char text[] = "on";
      radio.write(&text, sizeof(text));

      Serial.println(micros()-timer);
      
      delay(1000);
      const char text1[] = "off";
      radio.write(&text1, sizeof(text1));
      delay(1000);
  
}
