#include <Manchester.h>

/*

  Manchester Receiver example
  
  In this example receiver will receive one 16 bit number per transmittion

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

#define RX_PIN 11
#define LED_PIN 13

uint8_t moo = 1;
uint8_t data;
uint8_t id;
  
void setup() {
  
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);  
  digitalWrite(LED_PIN, moo);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive();
}

void loop() {
  if (man.receiveComplete()) { //received something
    uint16_t m = man.getMessage();
    
    man.beginReceive(); //start listening for next message right after you retrieve the message
    if (man.decodeMessage(m, id, data)) { //extract id and data from message, check if checksum is correct
      //id now contains ID of sender (or receiver(or any other arbitrary 4 bits of data))
      //data now contains one byte of data received from sender
      //both id and data are BELIEVED (not guaranteed) to be transmitted correctly    
      moo = ++moo % 2;
      digitalWrite(LED_PIN, moo);
    }
    
    
    //Convert uint to string
    char idstr[15];
    sprintf(idstr, "%d", id);
    char datastr[15];
    sprintf(datastr, "%d", data);
    
    //Print results
    Serial.print("Sensor ID: ");
    Serial.println(idstr);
    Serial.print("Data: ");
    Serial.println(datastr);
  
  }
}
