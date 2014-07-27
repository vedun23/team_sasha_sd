

void setup(){
 
    Serial.begin(115200);   
    delay(1000);
    
    Serial.println("Starting...");
    power_on();

    delay(3000);

    while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
        sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );


    sendATcommand("AT+CGSOCKCONT=1,\"IP\",\"m2m.com.attz\"", "OK", 2000);

}


void loop(){
 

  String variable = "GET /echo?value=test\r\n";
  httpRequest(variable);
   
   delay(5000);

}

void httpRequest(String request) {

  char url[ ]="108.244.165.72";
  int port= 5004;// or 5004, 5005
  int aux;
  int data_size = 0;

  char aux_str[100];

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
         answer = sendATcommand2("", "+CHTTPACT: DATA,", "+CHTTPACT: 0", 20000);
      } while (answer != 2); // Don't like this, it could get stuck.
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
    } while((x < length) && !timedOut);

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
