#include <phys253.h>
#include <LiquidCrystal.h>

int val;

void setup()
{
  #include <phys253setup.txt>
}

void loop(){

  val = analogRead(0);
  analogWrite(37, val / 64);
  LCD.clear();
  LCD.home();
  LCD.print("Analog IN:  "); LCD.print(val);
  LCD.setCursor(0,1);
  LCD.print("Analog OUT: "); LCD.print(val / 64);
  delay(250);
}
