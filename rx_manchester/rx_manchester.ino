//Libraries for Program

#include <Manchester.h>

/***Global Vars****/


//RX-TX Manchester

#define RX_PIN 11
#define LED_PIN 13

uint8_t moo = 1;
uint8_t data;
uint8_t id;

void setup(){
  
  Serial.begin(9600);
  
   //setup for the RX/TX
  pinMode(LED_PIN, OUTPUT);  
  digitalWrite(LED_PIN, moo);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive(); //listen for message from tx's
}

void loop(){
  
  
  //Start RX Logic
  if (man.receiveComplete()) //if there is a message received, process it
  {
    digitalWrite(LED_PIN, 1);
    uint16_t m = man.getMessage();
    man.beginReceive(); //start listening for next message after you retrieve message
    if (man.decodeMessage(m, id, data)) 
    { //extract id and data from message, check if checksum is correct
      //id now contains ID of sender (or receiver(or any other arbitrary 4 bits of data))
      //data now contains one byte of data received from sender
      //both id and data are BELIEVED (not guaranteed) to be transmitted correctly    
     // moo = ++moo % 2;
      //digitalWrite(LED_PIN, moo);
      
    }
    digitalWrite(LED_PIN,0);
    
    //Convert uint to string for processing
    char idstr[15];
    //sprintf(idstr, "%d", id);
    char datastr[15];
    //sprintf(datastr, "%d", data);
    
    itoa(data,datastr,10);
    itoa(id,idstr,10);
    
    String data1(datastr); //stores data
    String data2(idstr); //stores the id
    
    String variable = "GET /echo?id=" + data2 + "&data=" + data1 + "\r\n";
    Serial.print(variable);
}
}
