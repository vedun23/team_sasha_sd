/*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see .
 
*  Copyright (C) 2012 Libelium Comunicaciones Distribuidas S.L.
*  http://www.libelium.com
*  Version 0.1
*  Author: Alejandro Gállego
 
*  Version 0.1
*/
 
 
int led = 13;
int onModulePin = 2;        // the pin to switch on the module (without press on button) 
 
int x = 0;
 
char name[20];
 
void switchModule(){
    digitalWrite(onModulePin,HIGH);
    delay(2000);
    digitalWrite(onModulePin,LOW);
}
 
void setup(){
 
    Serial.begin(115200);                // UART baud rate
    delay(2000);
    pinMode(led, OUTPUT);
    pinMode(onModulePin, OUTPUT);
    switchModule();                    // switches the module ON
   
    for (int i=0;i < 5;i++){
        delay(5000);
        Serial.println(i);
    }   
 
    Serial.println("AT+CCAMS");     //starts the camera
    while(Serial.read()!='K');
 
    Serial.println("AT+CCAMSETD=640,480");     //sets VGA (640*480) resolution
    while(Serial.read()!='K');
 
    Serial.println("AT+FSLOCA=1");     //stores the image file in the 3G module
    while(Serial.read()!='K');
 
 
}
 
void loop(){
 
    delay(1500);
    while(Serial.available()!=0){
        Serial.read();
    }
    Serial.println("AT+CCAMTP");     //takes a picture, but not saved it
    while(Serial.read()!='K');  
 
    Serial.println("AT+CCAMEP");     // saves the picture into C:/Picture    
    Serial.flush();     
    while(Serial.read()!='/');
    while(Serial.read()!='/');
 
    x=0;
    do{
        while(Serial.available()==0);
        name[x]=Serial.read();
        x++;
    }while(x < 19);
 
    while(Serial.read()!='K');  
    Serial.println(name); 
 
    Serial.println("AT+CCAME");     // stops the camera
    while(Serial.read()!='K');  
 
    while(1);
 
}
