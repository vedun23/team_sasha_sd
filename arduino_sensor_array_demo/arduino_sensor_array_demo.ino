//Sensor Array Code
//Not Yet Tested

#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,5,4,3,2)
//Constants
const int buzzerPin = 0;
const int noteDuration = 1000/3;
const int photoVin = A5;
const int backgroundIRoffset = 20;
const int tempIn = A0;
//Values That Vary
int prevValue = 0;
int brightness = 0;
void setup()
{
  lcd.begin(16, 2);
  lcd.print("Demo Launched");
  delay(5000);
}

void loop()
{
  brightness = analogRead(photoVinPin) + backgroundIRoffset;
  int value = analogRead(tempIn);
  float celcius = (value /1024.0) * 5000 / 10;
  float fahrenheit = celcius * 9 / 5 + 32;
  if(brightness  > 10 && celcius > 15)
  {
  tone(buzzerPin, 20, noteDuration);
  noTone(buzzerPin);
  }
  lcd.setCursor(0,1);
  lcd.print("Brighness :");
  lcd.setCursor(0,2);
  lcd.print(brightness)
  delay(2000);
  lcd.clear();
  
  lcd.setCursor(0,1);
  lcd.print("Celcius :");
  lcd.setCursor(0,2);
  lcd.print(celcius)
  delay(2000);
  lcd.clear();
  
  lcd.setCursor(0,1);
  lcd.print("Fahrenheit :");
  lcd.setCursor(0,2);
  lcd.print(fahrenheit)
  delay(2000);
  lcd.clear();
  
}
