#include <SoftwareSerial.h>

int maxCelcius;

#define LED_PIN 0 //pin for blinking LED
#define tempPin A3
#define lightPin A2
#define RxD 1
#define TxD 2

#define DEBUG_ENABLED  1

int brightness = 0;
int temperature = 0;

//declaring character arrays
char tempChar[4];   
char brightChar[4]; 
String tStr; 
String bStr;

SoftwareSerial blueToothSerial(RxD,TxD);

void setup() {
  
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
    
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN,HIGH);
}

void loop() {
  //Code for reading from RX, if needed
  /* char recvChar;
  while(1){
    //check if there's any data sent from the remote bluetooth shield
    if(blueToothSerial.available()){
      recvChar = blueToothSerial.read();
      
        if(recvChar == '1')
          digitalWrite(led,HIGH);  
       
        else
          digitalWrite(led,LOW); 
    }
  }*/
  
    
    temperature = analogRead(tempPin) * 5000/1024/10;
    brightness = analogRead(lightPin);
    
    //conversions
    /*tStr=String(temperature); //converting integer into a string
    tStr.toCharArray(tempChar,4); //passing the value of the string to the character array
    
    bStr=String(brightness); //converting integer into a string
    bStr.toCharArray(brightChar,4); //passing the value of the string to the character array*/
    
    
    blueToothSerial.println(temperature);
    //delay(1000);
    blueToothSerial.println(brightness);
}

void setupBlueToothConnection()
{
  blueToothSerial.begin(9600); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=HC-05\r\n"); //set the bluetooth name as "HC-05"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  
  delay(2000); // This delay is required.
  //blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  blueToothSerial.print("Bluetooth connected!\n");
  
  delay(2000); // This delay is required.
  blueToothSerial.flush();
}
