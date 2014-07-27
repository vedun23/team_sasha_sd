#include <VirtualWire.h>
const int tx_pin = 1;
int Sensor1Data;
char Sensor1CharMsg[4]; 

void setup()
{
  pinMode(tx_pin, OUTPUT);
  vw_setup(2000);
  vw_set_tx_pin(tx_pin);
}

void loop()
{
    Sensor1Data = 45;
    itoa(Sensor1Data,Sensor1CharMsg,10);
    vw_send((uint8_t *)Sensor1CharMsg, strlen(Sensor1CharMsg));
    vw_wait_tx();
    
    delay(1000);
    
}
