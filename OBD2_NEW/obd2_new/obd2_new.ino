#include <SoftwareSerial.h>

SoftwareSerial obd(2,3);


char rxData[20];
char rxIndex=0;

//Variables to hold the speed and RPM data.
int vehicleSpeed=0;
int vehicleRPM=0;
void setup()
{
  obd.begin(9600);
  Serial.begin(9600);
  delay(1500);
  obd.println("ATZ");
  Serial.println("Establishing OBD-II-UART Connection");
  delay(2000);
  obd.flush();
  Serial.println("CONNECTION For OBD-II-UART Established");
}
void loop(){
  Serial.println("Flushing the OBD");
  obd.flush();
  Serial.println("OBD-II Flushed");
  obd.println("010D");
  Serial.println("Getting Response");
  //Get the response from the OBD-II-UART board. We get two responses
  //because the OBD-II-UART echoes the command that is sent.
  //We want the data in the second response.
  getResponse();
  getResponse();
  //Convert the string data to an integer
  vehicleSpeed = strtol(&rxData[6],0,16);
  //Print the speed data to the lcd
  Serial.print(vehicleSpeed);
  Serial.print(" km/h\r\n");
  delay(100); 
  Serial.println("Flushing the OBD");
  obd.flush();
  Serial.println("OBD-II Flushed");

  Serial.println("010C");
  obd.println("010C");
  
  //Get the response from the OBD-II-UART board
  getResponse();
  getResponse();
  //Convert the string data to an integer
  //NOTE: RPM data is two bytes long, and delivered in 1/4 RPM from the OBD-II-UART
  vehicleRPM = ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16))/4;
  //Print the rpm data to the lcd
  Serial.print(vehicleRPM); 
  Serial.println(" RPM\r\n"); 
  
  //Give the OBD bus a rest
  delay(100);

}
//The getResponse function collects incoming data from the UART into the rxData buffer
// and only exits when a carriage return character is seen. Once the carriage return
// string is detected, the rxData buffer is null terminated (so we can treat it as a string)
// and the rxData index is reset to 0 so that the next string can be copied.
void getResponse(void){
  char inChar=0;
  //Keep reading characters until we get a carriage return
  while(inChar != '\r'){
    //If a character comes in on the serial port, we need to act on it.
    if(obd.available() > 0){
      //Start by checking if we've received the end of message character ('\r').
      if(obd.peek() == '\r'){
        //Clear the Serial buffer
        inChar=obd.read();
        //Put the end of string character on our data string
        rxData[rxIndex]='\0';
        //Reset the buffer index so that the next character goes back at the beginning of the string.
        rxIndex=0;
      }
      //If we didn't get the end of message character, just add the new character to the string.
      else{
        //Get the new character from the Serial port.
        inChar = obd.read();
        //Add the new character to the string, and increment the index variable.
        rxData[rxIndex++]=inChar;
      }
    }
  }
}
