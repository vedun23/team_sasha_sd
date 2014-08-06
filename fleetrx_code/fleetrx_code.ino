//Libraries for Program

#include <Manchester.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Servo.h> 
#include <MemoryFree.h>

String vin = "JM3LW28A230055355";

/***Global Vars****/
//Demo control options
boolean obdON = true;
boolean gpsON = true;

//declare servo
Servo servo1;
int pos = 0;    // variable to store the servo position

//These all print to http req

String variable; // holds the request with all variable data

String sensor1Temp;
String sensor1Light;

String sensor2Temp;
String sensor2Light;

String sensor3gas;

//RX-TX Manchester
#define BUFFER_SIZE 3
#define RX_PIN 11
#define LED_PIN 13
#define TX_LEDPIN 14

//temp reading buffer
uint8_t buffer[BUFFER_SIZE];
uint8_t data[BUFFER_SIZE];

//OBD serial port
SoftwareSerial obd(9,10);

//GPS global vars
int gpsAnswer = 0;
String latie; //print to http req
String longie; //print to http req

if(!gpsON){
	//TODO declare float array here for these to cycle through on a route
	latie ="3259.59816"; 
	longie="09645.15992";
}

//for data timeouts, keeps data from becoming stale
uint8_t stageOneCounter = 0; //rxtx timeout
uint8_t stageTwoCounter = 0; //gps timeout
uint8_t stageThreeCounter = 0; //obd2 timeout

uint8_t sensorCache[4]; // stores cached data between sensor reads


//OBD global vars
//This is a character buffer that will store the data from the serial port
char rxData[20];
char rxIndex=0;

String vehRpm; //print to http req
String vehSpeed; //print to http req

if(!obdON){
	vehRpm = 1000; //defaults if obd is off
	vehSpeed = 60; //defaults if obd is off
}


//Global Flags for logic control
boolean rxFlag1 = false;
boolean rxFlag2 = false;
boolean rxFlag3 = false;

//Initialize control flags
boolean stageOne = false; //RXTX always runs
boolean stageTwo = false;
boolean stageThree = false;

//set control flags if we are not running gps or obd modules
if(!obdON){
	stageThree = true;
}

if(!gpsON){
	stageTwo = true;
}



/****Setup Code ****/
void setup() {

   myservo.attach(5);  // attaches the servo on pin 9 to the servo object 
     
   cellStart();
   Serial.flush();
   
   if(gpsON){
      gpsStart();
	  Serial.flush();
   }
    
   rxTxStart();
   Serial.flush();
   
   if(obdON){
	   obdStart();
	   Serial.flush();
   }
  
   Serial.print(F("Free Memory = "));
   Serial.println(getFreeMemory());
  
   Serial.flush();
  
}

void loop() {
  
  
   //receive data from rx/tx 434 Mhz and GPS simultaneously
   //Serial.println(F("Starting Stage One"));
   if(!stageOne){
     
       stageOne =  rxLoop();
       Serial.flush();
   }
   
   if(stageOne && !stageTwo)
   {
     if(gpsON){	 
		stageTwo = gpsLoop();
		Serial.flush();
	}
   } 
   
    if((stageOne && stageTwo) == true)
    {
		if(obdON){
			 stageThree = obdLoop();
			 Serial.flush();
		 }    
              if(stageThree == true)
              {
                     Serial.print(F("Free Memory = "));
                     Serial.println(getFreeMemory());
                    //Send a complete data string to Breeze via 3G
                    variable = "GET /echo?&vin=" + vin + "&tmp1=" + sensor1Temp + "&bri1=" + sensor1Light + "&tmp2=" + sensor2Temp + "&bri2=" + sensor2Light  + "&rpm=" + vehRpm + "&spd=" + vehSpeed + "&lat=" + latie + "&lng=" + longie + "&co=" + sensor3gas + "\r\n";
					
					//send data to cloud
                    httpRequest(variable);
					
                    //reset control flags
                    stageOne = false;
					if(gpsON){
						stageTwo = false;
					}
					
					if(obdON){
						stageThree = false;
					}
					
                    Serial.flush();
                    delay(5000);             
              }
              else
              {       
              }
    }
    else
    {
    }
}

 //TODO COMMENT OUT SERIAL PRINTS
void httpRequest(String request) {
  int aux;
  int data_size = 0;
  char url[ ]="108.244.165.72";
  int port= 5004;// or 5004, 5005
  

  char aux_str[50];
  char data[600];
  int x = 0;
  int8_t answer;
  // SETUP the url
  sprintf(aux_str, "AT+CHTTPACT=\"%s\",%d", url, port);
  
  Serial.flush();

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

            Serial.print(F("Data to receive: "));
            Serial.println(data_size);

            if (data_size > 0) 
            {
               int returned = readChars(data, data_size < sizeof(data) ? data_size : sizeof(data), 10000);
               Serial.print(F("Returned: "));
               Serial.print(returned);
               Serial.print(F(" : "));
               
			   if(data.indexOf('ALARM') != -1)
			   {
						Serial.println(F("ALARM RECIEVED"));
						//turn servo on alarm
						for(pos = 0; pos < 180; pos += 1)  // goes from 0 degrees to 180 degrees 
						{                                  // in steps of 1 degree 
							myservo.write(pos);              // tell servo to go to position in variable 'pos' 
							delay(15);                       // waits 15ms for the servo to reach the position 
						} 
						
						for(pos = 180; pos>=1; pos-=1)     // goes from 180 degrees to 0 degrees 
						{                                
							myservo.write(pos);              // tell servo to go to position in variable 'pos' 
							delay(15);                       // waits 15ms for the servo to reach the position 
						}  	
			   }
			   else{
						Serial.println(data);
			   }
            }
         }
         else {
            Serial.println(F("Error getting the url"));
            data_size = 0;

            break;
         }
                    // Look for end of data
         answer = sendATcommand2("","+CHTTPACT: DATA,", "+CHTTPACT: 0", 20000);
       
      } while (answer !=2); // Don't like this, it could get stuck.
    }
    else {
       Serial.println(F("Error writing the request"));
    }

}

//COMMENT OUT SERIAL PRINTS
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
    } while((x < length) && !timedOut);

    buffer[x] = 0; // terminate string

    if (timedOut){
      long howLong = millis() - previous;
      Serial.println(F("Timed out reading response"));
      Serial.print(F("Expected: "));
      Serial.print (length);
      Serial.print (F(", Got: "));
      Serial.println(x);
      Serial.print(F(", Took (ms): "));
      Serial.println(howLong);
      Serial.print(F("Timeout passsed in: "));
      Serial.println(timeout);
    }    
    return x;
}

void power_on(){
 
   int onModulePin = 2;

    uint8_t answer=0;
    
    // checks if the module is started
    answer = sendATcommand("AT", "OK", 1000);
    if (answer == 0)
    {
        // power on pulse
        digitalWrite(onModulePin,HIGH);
        delay(3000);
        digitalWrite(onModulePin,LOW);
    
        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand("AT", "OK", 1000);    
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
    Serial.flush();

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

int8_t sendATcommand2(char* ATcommand, char* expected_answer1,
         char* expected_answer2, unsigned int timeout)
{
    char response[100];
    uint8_t x=0,  answer=0;

    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 
    Serial.flush();

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
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }
        // Waits for the asnwer with time out
    }
    while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
}



int8_t sendATcommand3(char* ATcommand, char* expected_answer1, unsigned int timeout)
{

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 
      Serial.flush();

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

//start GPS module
void gpsStart(){
  
  Serial.println(F("GPS Starting"));
  Serial.flush();
  // starts GPS session in stand alone mode
   gpsAnswer = sendATcommand("AT+CGPS=1,1","OK",1000);    
   if (gpsAnswer == 0)
    {
        Serial.println(F("Error starting the GPS"));
        while(1);
   }
   
   Serial.println(F("GPS Init finished"));
   Serial.flush();
   delay(5000);
}

void cellStart(){

    //open serial port
    Serial.begin(115200);
   
   //setup for cellular/3g shield
    delay(2000);
    Serial.println(F("3G Shield Starting..."));
    power_on();
    delay(5000);
  
   while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );
   sendATcommand("AT+CGSOCKCONT=1,\"IP\",\"m2m.com.attz\"", "OK", 2000);
   
   Serial.println(F("3G Init Finished"));
   Serial.flush();
}

void obdStart(){
  
  obd.begin(9600);
  //Serial.begin(9600);
  delay(1500);
  obd.println("ATZ");
  Serial.println(F("Establishing OBD-II-UART Connection"));
  Serial.flush();
  delay(2000);
  obd.flush();
  Serial.println(F("CONNECTION For OBD-II-UART Established"));
  Serial.flush();
}

void rxTxStart(){
  
  Serial.println(F("RXTX Starting"));
  Serial.flush();
  
  pinMode(LED_PIN, OUTPUT); 
  pinMode(TX_LEDPIN, OUTPUT); 
  digitalWrite(LED_PIN, 1);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE,buffer); //listen for message from tx's
  
  Serial.println(F("RXTX Init Finished"));
    Serial.flush();
}

boolean rxLoop(){
 
      //RX/TX vars
      uint8_t id; // id of the sensor
      uint8_t data1; //first data point
      uint8_t data2; //second data point
     
      char tempStr[5];
      char lightStr[5];
      
      char tempstr2[5];
      char lightstr2[5];

      char gasStr[5];
    
      if(!(rxFlag1 && rxFlag2 && rxFlag3))
      {
        //Serial.println(F("Rx Searching..."));
        if(man.receiveComplete())//if there is a message received
         {  
            digitalWrite(TX_LEDPIN, 1);  
            //copies received data to the buffer
            //holds received id
            id = buffer[0];
            //holds received temp
            data1  = buffer[1];
            //holds received light data
            data2 = buffer[2];
			
            man.beginReceiveArray(BUFFER_SIZE,buffer);
            digitalWrite(TX_LEDPIN,0);
            delay(5000);
            
            //Store different data in separate sensor caches
			
            //Sensor 1 cache (Temp/Light data)
            if(id == 1)
            {
              Serial.println(F("Got sensor 1 data"));
              Serial.flush();
			  
              sensorCache[0] = data1;
              sensorCache[1] = data2;
			  
              if((data1 != 0) && (data2!=0) )
              {
                rxFlag1 = true;
              }
              
            }
            //Sensor 2 cache (Temp/Light data)
            if(id == 2)
            {
              Serial.println(F("Got sensor 2 data"));
              Serial.flush();
			  
              sensorCache[2] = data1;
              sensorCache[3] = data2;
			  
              if((data1 != 0) && (data2!=0) )
              {
                rxFlag2 = true;
              }
			  
             }
             //Sensor 3 cache (O2 Gas Sensor)
            if(id == 3)
            {
              Serial.println(F("Got sensor 3 data"));
			  Serial.flush();
			  
              sensorCache[4] = data1;

              if(data1!=0)
              {
                rxFlag3 = true;
              }
             }
   
            //Convert integers to char array
            itoa(sensorCache[0],tempStr,10);
            itoa(sensorCache[1],lightStr,10);
            itoa(sensorCache[2],tempStr2,10);
            itoa(sensorCache[3],lightStr2,10);
            itoa(sensorCache[4],gasStr,10);
            
			
            String string1(tempStr); //stores the temp
            sensor1Temp = string1;
            sensor1Temp[2] = '\0'; 
			
            String string2(lightStr); //stores the light sensed data
            sensor1Light = string2;
						
            String string3(tempStr2); //stores temp 2
            sensor2Temp = string3;
            sensor2Temp[2] = '\0';
			
            String string4(lightStr2); //stores light sensed data 2
            sensor2Light = string4;
			
            String string5(gasStr); //stores light sensed data 2
            sensor3gas = string5;

            Serial.println(F("RX IS FINISHED."));
            Serial.flush();
            return false;
         }      
      }
      
      else if(rxFlag1 && rxFlag2 && rxFlag3)
      {
         Serial.println(F("ALL RX DATA RECEIVED"));   
         Serial.flush();
		 
         rxFlag1 = false;
         rxFlag2 = false;
         rxFlag3 = false;
		 
         return true;
       }  
}



boolean obdLoop(){
  
  //Variables to hold the speed and RPM data.
  int vehicleSpeed=0;
  int vehicleRPM=0;
  
  char speedStr[5];
  char rpmStr[5];
  int counter = 0;
 
   do
   {
      //Serial.println(F("Flushing the OBD"));
      obd.flush();
      //Serial.println(F("OBD-II Flushed"));
      obd.println("010D");
      Serial.println(F("Getting OBD-II Response"));
      //Get the response from the OBD-II-UART board. We get two responses
      //because the OBD-II-UART echoes the command that is sent.
      //We want the data in the second response.
      getResponse();
      getResponse();
      //Convert the string data to an integer
      vehicleSpeed = strtol(&rxData[6],0,16);
      //Print the speed data to the lcd
      //Serial.print(vehicleSpeed);
      //Serial.print(" km/h\r\n");
      delay(100); 
      //Serial.println(F("Flushing the OBD"));
      obd.flush();
      //Serial.println(F("OBD-II Flushed"));
    
      //Serial.println(F("010C"));
      obd.println("010C");
      
      //Get the response from the OBD-II-UART board
      getResponse();
      getResponse();
      //Convert the string data to an integer
      //NOTE: RPM data is two bytes long, and delivered in 1/4 RPM from the OBD-II-UART
      vehicleRPM = ((strtol(&rxData[6],0,16)*256)+strtol(&rxData[9],0,16))/4;
      //Print the rpm data to the lcd
      //Serial.print(vehicleRPM); 
      //Serial.println(" RPM\r\n"); 
      
      //Give the OBD bus a rest
      delay(100);
      
      //convert vehicle RPM /speed to strings
      //Convert integers to char array
      itoa(vehicleSpeed,speedStr,10);
      itoa(vehicleRPM,rpmStr,10);
      
      String string1(speedStr); //stores id
      vehSpeed =string1;//Serial.println(sensor1Id);
      String string2(rpmStr); //stores the temp
      vehRpm = string2;  
      counter++;
  
   } while((vehicleSpeed == 0 || vehicleRPM == 0) && counter != 5);
   
   return true;
}

boolean gpsLoop(){
  
  char gps_data[100];
  int counter;
  int N;
  int S;
  int E;
  int W;
  
  //actual array locations for N/S E/W
  int gps1;
  int gps2;
     
        gpsAnswer = sendATcommand3("AT+CGPSINFO","+CGPSINFO:",1000);    // request info from GPS
          if (gpsAnswer == 1)
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
                     Serial.println(F("No GPS data currently available"));  
                     Serial.flush();
              }
              else
              {         
                //convert char array to string
                 String gpsData(gps_data);
                 //Serial.println(gps_data);   
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
               Serial.println(F("Error gathering GPS data...no answer."));
               Serial.flush(); 
               return false;
          }
}


