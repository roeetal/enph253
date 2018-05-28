#include <phys253.h>
#include <LiquidCrystal.h>

void setup()
{
  #include <phys253setup.txt>
}

float left = 0;
float right = 0;
int count = 1;
int sm = 2;
int limit = 150;
#define LEFT 0
#define RIGHT 1 
int right_speed = 200;
int left_speed = 200;

void loop(){
  if(count % sm != 0){
    left += analogRead(LEFT);
    right += analogRead(RIGHT);
    count++;
  }else{
    left += analogRead(LEFT);
    right += analogRead(RIGHT);
    left/=sm;
    right/=sm;
    if(right>=limit && left<limit){
        right_speed -= 50;
    }else if(right<limit && left>=limit){
        left_speed -= 50;
    }else{
      right_speed = 200;
      left_speed = 200;
    }
    motor.speed(RIGHT,right_speed);
    motor.speed(LEFT,left_speed);
    count++;
  }
    LCD.clear();
    LCD.home();
    LCD.setCursor(0,0);
    LCD.print(left);
    LCD.setCursor(0,1);
    LCD.print(right);
}

