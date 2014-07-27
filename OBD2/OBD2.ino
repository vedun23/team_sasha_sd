#include <SoftwareSerial.h>


//Create an instance of the new soft serial library to control the serial LCD
//Note, digital pin 3 of the Arduino should be connected to Rx of the serial LCD.

SoftwareSerial obd(2,3);

//This is a character buffer that will store the data from the serial port
char rxData[20];
char rxIndex=0;

//Variables to hold the speed and RPM data.
int vehicleSpeed=0;
int vehicleRPM=0;

void setup(){
  //Both the Serial LCD and the OBD-II-UART use 9600 bps.
  obd.begin(9600);
  Serial.begin(9600);
  //Serial.println("SERIAL STARTED");
  //Clear the old data from the LCD.
 
 
  //Wait for a little while before sending the reset command to the OBD-II-UART
  delay(1500);
  //Reset the OBD-II-UART
  obd.println("ATZ");
  //Wait for a bit before starting to send commands after the reset.
  delay(2000);

  //Delete any data that may be in the serial port before we begin.
  Serial.flush();
}

void loop(){
  //Delete any data that may be in the serial port before we begin.  
  obd.flush();
  //Set the cursor in the position where we want the speed data.
  //Query the OBD-II-UART for the Vehicle Speed
  obd.println("010D");
  //Get the response from the OBD-II-UART board. We get two responses
  //because the OBD-II-UART echoes the command that is sent.
  //We want the data in the second response.
  getResponse();
  getResponse();
  //Convert the string data to an integer
  vehicleSpeed = strtol(&rxData[6],0,16);
  //Print the speed data to the lcd
  Serial.println(vehicleSpeed);
  //Serial.print(" km/h");
  delay(100);

  //Delete any data that may be left over in the serial port.
  obd.flush();
  //Move the serial cursor to the position where we want the RPM data.
  //Clear the old RPM data, and then move the cursor position back.
  
  //Query the OBD-II-UART for the Vehicle rpm
  obd.println("010C");
  //Get the response from the OBD-II-UART board
  getResponse();
  getResponse();
  //Convert the string data to an integer
  //NOTE: RPM data is two bytes long, and delivered in 1/4 RPM from the OBD-II-UART
  vehicleRPM = ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16))/4;
  //Print the rpm data to the lcd
  Serial.println(vehicleRPM); 

  //Give the OBD bus a rest
  delay(100);
  
    char idstr[15];
    char datastr[15];
  
    itoa(vehicleRPM,datastr,10);
    itoa(vehicleSpeed,idstr,10);
  
    String data1(datastr); //stores data
    String data2(idstr); //stores the id
    
    String variable = "GET /echo?id=3";
    variable += "&rpm=";
    variable += data1;
    variable +="&speed=";
    variable += data2;
    variable += "\r\n";
    Serial.print(variable);

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
