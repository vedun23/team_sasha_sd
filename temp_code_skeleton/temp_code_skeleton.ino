#define tempPin = A0

int temperature;
int maxCelcius;

void.setup()
{
  Serial.begin(9600);
  Serial.println("Enter Max Temperature Range In Celcius");
  while (Serial.available() == 0)
  {
    maxCelcius = Serial.parseInt();
    Serial.println("Connection Has Been Established. Polling the bridge");
    Serial.flush();
  }
}
void loop()
{
  temperature = analogRead(tempPin) * 5000/1024/10;
  Serial.println(temperature);
  if(temperature >= maxCelcius)
  {
  }
}


