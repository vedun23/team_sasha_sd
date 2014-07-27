#include <VirtualWire.h>
int Sensor1Data;
char Sensor1CharMsg[4]; 
const int rx_pin = 11;
void setup()
{
  Serial.begin(9600);
  Serial.println("Setup");
  vw_set_rx_pin(rx_pin);
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_rx_start();
}
void loop()
{
  //Serial.println("Entered Loop"); 
  uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)) 
    {
		int i;
                 digitalWrite(13, true); 
   		Serial.print("Got: ");
	
		for (i = 0; i < buflen; i++)
		{
			 Sensor1CharMsg[i] = char(buf[i]);
		}
Sensor1CharMsg[buflen] = '\0';
Sensor1Data = atoi(Sensor1CharMsg);
 Serial.print("Sensor 1: ");
        Serial.println(Sensor1Data);
         digitalWrite(13, false);
        		
	}
}
