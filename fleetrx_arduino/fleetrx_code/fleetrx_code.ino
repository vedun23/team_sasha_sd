//Libraries for Program

#include <Manchester.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>

/***Global Vars****/
boolean en
boolean enter2 = false;

//RX/TX strings
uint8_t sensorCache[6]; // stores cached data between sensor reads
uint8_t id;
uint8_t temp;
uint8_t light;
//Convert ints to string for processing
char idstr[5];
char tempstr[5];
char lightstr[5];

char idstr2[5];
char tempstr2[5];
char lightstr2[5];

String sensor1Id;
String sensor1Temp;
String sensor1Light;

String sensor2Id;
String sensor2Temp;
String sensor2Light;

//RX-TX Manchester
#define BUFFER_SIZE 3
#define RX_PIN 11
#define LED_PIN 13

//temp reading buffer
uint8_t buffer[BUFFER_SIZE];
uint8_t data[BUFFER_SIZE];

//OBD serial port
SoftwareSerial obd(2,3);

//GPS global vars
char gps_data[25];
uint8_t counter;
uint8_t answer;
uint8_t N;
uint8_t S;
uint8_t E;
uint8_t W;

//actual array locations for N/S E/W
uint8_t gps1;
uint8_t gps2;
String latie;
String longie;

//OBD global vars
String vehRPM;
String vehSpeed;
//This is a character buffer that will store the data from the serial port
char rxData[20];
char rxIndex=0;
//Variables to hold the speed and RPM data.
int vehicleSpeed=0;
int vehicleRPM=0;
char vroom[8];
char vspeed[8];

//Global Flags for logic control
boolean rxFlag1 = false; //toggle sensor 1
boolean rxFlag2 = false; //toggle sensor 2
boolean obdFlag = true; //toggle obd

/****Setup Code ****/
void setup() {

  cellStart();

  //obdStart();

  gpsStart();

  rxtxStart();

}

void loop() {
  //receive data from rx/tx 434 Mhz
  enter =  rxLoop();

  if(enter == 1)
  {
    //obdLoop();

    //Poll GPS if we got obd2 data
    if(obdFlag == 1)
    {
      enter2 = gpsLoop();

      if(enter2 == 1)
      {

        //Send a complete data string to Breeze via 3G
        String variable = "GET /echo?id=" +sensor1Id+ "&tmp=" + sensor1Temp + "&btness=" + sensor1Light + "id=" + sensor2Id + "&tmp=" + sensor2Temp + "&btness=" + sensor2Light + "&lat=" + latie + "&long=" + longie + "\r\n";
        httpRequest(variable);
        delay(5000);
      }   
      else
      {

        Serial.println("No GPS Data Available");
      }            
    }   
    else
    {
      Serial.println("Incomplete OBD Data");
    }
  }
  else
  {
    Serial.println("Incomplete RF Sensor Data");
  }
}


void httpRequest(String request) {
  int aux;
  int data_size = 0;
  char url[ ]="108.244.165.72";
  int port= 5004;// or 5004, 5005

  char aux_str[50];
  char data[512];
  int x = 0;
  int8_t answer;
  // SETUP the url
  sprintf(aux_str, "AT+CHTTPACT=\"%s\",%d", url, port);

  answer = sendATcommand(aux_str, "+CHTTPACT: REQUEST", 10000);

  if (answer == 1) {

    Serial.println(request);
    // Sends <Ctrl+Z>
    aux_str[0] = 0x1A;
    aux_str[1] = 0x00;

    // Now will get back length of data expected
    answer = sendATcommand(aux_str, "+CHTTPACT: DATA,", 10000);

    x = 0;
    do {
      if (answer == 1) 
      {
        data_size = 0;
        while (Serial.available() == 0);

        aux = Serial.read();
        do {
          data_size *= 10;
          data_size += (aux - 0x30);
          while (Serial.available() == 0);
          aux = Serial.read();
        }

        while (aux != 0x0D);

        // Now get the LF
        while (Serial.available()==0);
        aux = Serial.read();

        Serial.print("Data to receive: ");
        Serial.println(data_size);

        if (data_size > 0) 
        {
          int returned = readChars(data, data_size < sizeof(data) ? data_size : sizeof(data), 10000);
          Serial.print("Returned: ");
          Serial.print(returned);
          Serial.print(" : ");
          Serial.println(data);
        }
      }
      else {
        Serial.println("Error getting the url");
        data_size = 0;

        break;
      }

      // Look for end of data
      answer = sendATcommand("+CHTTPACT: DATA,", "+CHTTPACT: 0", 20000);
    } 
    while (answer != 2); // Don't like this, it could get stuck.
  }
  else {
    Serial.println("Error waiting the request");
  }

}

int readChars(char *buffer, int length, int timeout){
  int x = 0;
  long lTimeout = timeout;
  long previous = millis();
  boolean timedOut = false;

  if (length <= 0) return 0;

  // this loop waits for the answer
  do{
    while (Serial.available() != 0){    
      buffer[x] = Serial.read();
      // Serial.print(buffer[x]);
      x++;
    }
    // Waits for the asnwer with time out
    timedOut = ((millis() - previous) > lTimeout) ? true : false;
  } 
  while((x < length) && !timedOut);

  buffer[x] = 0; // terminate string

  if (timedOut){
    long howLong = millis() - previous;
    Serial.println("Timed out reading response");
    Serial.print("Expected: ");
    Serial.print (length);
    Serial.print (", Got: ");
    Serial.println(x);
    Serial.print(", Took (ms): ");
    Serial.println(howLong);
    Serial.print("Timeout passsed in: ");
    Serial.println(timeout);
  }    
  return x;
}

void power_on(){

  uint8_t onModulePin = 2;
  pinMode(onModulePin, OUTPUT);
  uint8_t answer=0;

  // checks if the module is started
  answer = sendATcommand("AT", "OK", 2000);
  Serial.print("power on answer "); 

  if (answer == 0)
  {
    // power on pulse
    digitalWrite(onModulePin,HIGH);

    delay(3000);
    digitalWrite(onModulePin,LOW);


    // waits for an answer from the module
    while(answer == 0){    
      // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 2000); 
      Serial.print("power on"); 

    }
  }

}


int8_t sendATcommand(char* ATcommand, char* expected_answer1,
unsigned int timeout)
{
  char response[100];
  uint8_t x=0,  answer=0;

  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  while( Serial.available() > 0) Serial.read();    // Clean the input buffer

  Serial.println(ATcommand);    // Send the AT command 


    x = 0;
  previous = millis();

  // this loop waits for the answer
  do{

    if(Serial.available() != 0){    
      response[x] = Serial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer1) != NULL)    
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  }
  while((answer == 0) && ((millis() - previous) < timeout));    

  return answer;
}

//The getResponse function collects incoming data from the UART uint8_to the rxData buffer
// and only exits when a carriage return character is seen. Once the carriage return
// string is detected, the rxData buffer is null terminated (so we can treat it as a string)
// and the rxData index is reset to 0 so that the next string can be copied.
void getResponse(){

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

//start GPS module
void gpsStart(){

  Serial.println("GPS Starting");

  uint8_t answer;

  // starts GPS session in stand alone mode
  answer = sendATcommand("AT+CGPS=1,1","OK",1000);    
  if (answer == 0)
  {
    Serial.println("Error starting the GPS");
    while(1);
  }

  Serial.println("GPS Init finished");

}

void cellStart(){

  //open serial port
  Serial.begin(115200);

  //setup for cellular/3g shield
  delay(2000);
  Serial.println("3G Shield Starting...");
  power_on();
  delay(5000);

  while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );
  sendATcommand("AT+CGSOCKCONT=1,\"IP\",\"m2m.com.attz\"", "OK", 2000);

  Serial.println("3G Init Finished");
}

void obdStart(){

  Serial.println("OBD Starting");
  //Both the Serial LCD and the OBD-II-UART use 9600 bps.
  obd.begin(9600);

  //Reset the OBD-II-UART
  obd.println("ATZ");
  delay(2000);
  Serial.println("OBD Init Finished");
}

void rxtxStart(){

  Serial.println("RXTX Starting");

  pinMode(LED_PIN, OUTPUT);  
  digitalWrite(LED_PIN, 1);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE,buffer); //listen for message from tx's

  Serial.println("RXTX Init Finished");
}

boolean rxLoop(){

  if(!(rxFlag1 && rxFlag2))
  {
    Serial.println("Rx Searching...");
    if(man.receiveComplete())
    { 
      Serial.println("Got some sensor data");
      //if there is a message received

      digitalWrite(LED_PIN, 1);  
      //copies received data to the buffer
      man.beginReceiveArray(BUFFER_SIZE,buffer);
      delay(5000);
      //holds received id
      id = buffer[0];
      //holds received temp
      temp  = buffer[1];
      //holds received light data
      light = buffer[2];

      digitalWrite(LED_PIN,0);

      //Store data in sensor caches
      //Sensor 1 cache
      if(id == 1)
      {
        sensorCache[0] = id;
        sensorCache[1] = temp;
        sensorCache[2] = light;
        rxFlag1 = true;
      }
      //Sensor 2 cache
      if(id == 2)
      {
        sensorCache[3] = id;
        sensorCache[4] = temp;
        sensorCache[5] = light;
        rxFlag2 = true;
      }

      //Convert uint8_tegers to char array
      itoa(sensorCache[0],idstr,10);
      itoa(sensorCache[1],tempstr,10);
      itoa(sensorCache[2],lightstr,10);
      itoa(sensorCache[3],idstr2,10);
      itoa(sensorCache[4],tempstr2,10);
      itoa(sensorCache[5],lightstr2,10);

      //Convert char array to strings
      String sensor1Id(idstr); //stores id
      //Serial.println(sensor1Id);
      String sensor1Temp(tempstr); //stores the temp
      // Serial.println(sensor1Temp);
      String sensor1Light(lightstr); //stores the light sensed data
      //Serial.println(sensor1Light);
      String sensor2Id(idstr2); //stores id 2
      // Serial.println(sensor2Id);
      String sensor2Temp(tempstr2); //stores temp 2
      // Serial.println(sensor2Temp);
      String sensor2Light(lightstr2); //stores light sensed data 2
      // Serial.println(sensor2Light);
    }

  }
  else if(rxFlag1 && rxFlag2){

    return true;
  }
  else
  {
    return false;
  }        
}



void obdLoop(){

  Serial.println("OBD Loop Started");

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
  //Convert the string data to an uint8_teger
  vehicleSpeed = strtol(&rxData[6],0,16);
  //Pruint8_t the speed data to the lcd
  Serial.println(vehicleSpeed);
  //Serial.pruint8_t(" km/h");
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
  //Convert the string data to an uint8_teger
  //NOTE: RPM data is two bytes long, and delivered in 1/4 RPM from the OBD-II-UART
  vehicleRPM = ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16))/4;
  //Print the rpm data to the lcd
  Serial.println(vehicleRPM); 

  //Give the OBD bus a rest
  delay(100);


  itoa(vehicleRPM,vroom,10);
  itoa(vehicleSpeed,vspeed,10);

  String vehRPM(vroom); //stores RPM
  String vehSpeed(vspeed); //stores the speed

    //if we got non-zero data, set logic flag true
  if(vroom[0] != 0 && vspeed[0] != 0)
  {   
    Serial.println("Got good OBD2 data");
    obdFlag = true;
  }

  Serial.println("OBD Loop end");

}

boolean gpsLoop(){

  answer = sendATcommand("AT+CGPSINFO","+CGPSINFO:",1000);    // request info from GPS
  if (answer == 1)
  {
    counter = 0;
    do{
      while(Serial.available() == 0);
      gps_data[counter] = Serial.read();
      counter++;
    }
    while(gps_data[counter - 1] != '\r');
    gps_data[counter] = '\0';

    if(gps_data[0] == ',')
    {
      //Serial.println("No GPS data available");  
      return false;
    }
    else
    {
      //Serial.pruuuint8_t8_t8_t("GPS data:");
      // Serial.pruuuint8_t8_t8_tln(gps_data);

      //convert char array to string
      String gpsData(gps_data);

      N = gpsData.indexOf("N");
      S = gpsData.indexOf("S");
      W = gpsData.indexOf("W");
      E = gpsData.indexOf("E");

      if(N != -1)
      {
        gps1 = N;
      }
      if(S != -1)
      {
        gps1 = S;
      }  
      if(E != -1)
      {
        gps2 = E;
      }  
      if(W != -1)
      {
        gps2 = W;
      } 

      latie = gpsData.substring(0,gps1-2); //stores lat
      longie = gpsData.substring(gps1+2,gps2-2); //stores long
      return true;
    }
  }
  else
  {
    Serial.println("Error"); 
  }
}

