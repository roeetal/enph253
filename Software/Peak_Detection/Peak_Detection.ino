#include <phys253.h>
#include <LiquidCrystal.h>

void setup()
{
  #include <phys253setup.txt>
}

int analog;

void loop(){
    analog = analogRead(0);
    LCD.clear();
    LCD.home();
    LCD.setCursor(0,0);
    LCD.print(analog);
    delay(100);
}

