#include <Manchester.h>

int sensorValue;

/*

  Manchester Transmitter example
  
  In this example transmitter will send one 16 bit number per transmittion

  try different speeds using this constants, your maximum possible speed will 
  depend on various factors like transmitter type, distance, microcontroller speed, ...

  MAN_300 0
  MAN_600 1
  MAN_1200 2
  MAN_2400 3
  MAN_4800 4
  MAN_9600 5
  MAN_19200 6
  MAN_38400 7
*/

long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 19000;           // interval at which to exe (milliseconds)

#define TX_PIN 1  //pin where your transmitter is connected
#define LED_PIN 0 //pin for blinking LED
#define SENDER_ID 3 //Sensor ID changes as you program different Tiny's

//uint8_t moo = 0; //last led status
uint16_t transmit_data = 0;

//RX Array Data
uint8_t data[3];
void setup() {
    
  pinMode(LED_PIN, OUTPUT);
 // digitalWrite(LED_PIN, moo);
  man.workAround1MhzTinyCore(); //add this in order for transmitter to work with 1Mhz Attiny85/84
  man.setupTransmit(TX_PIN, MAN_1200);
}

void loop() {

  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   

     sensorValue = analogRead(3);       // read analog input pin 0
     Serial.println(sensorValue, DEC);  // prints the value read
    
    data[0] = SENDER_ID; //Sender ID
    data[1] = sensorValue;
    data[3] = 0;
  //Serial.println(temperature);
  //man.transmit(temperature);
  digitalWrite(LED_PIN,1);
  man.transmitArray(3, data);
  digitalWrite(LED_PIN, 0);
  //transmit_data++;
  }
 
}
