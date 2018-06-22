#include <phys253.h>
#include <LiquidCrystal.h>

void setup()
{
  #include <phys253setup.txt>
}
#define LEFT 0
#define RIGHT 1

float left, right = 0;
float left_error, right_error = 0;
float last_left_error, last_right_error = 0;
float g_right, g_left = 0;
float maxi_right, maxi_left = 10;
float i_left, i_right = 0;
float d_right, d_left = 0;
float p_right, p_left = 0;
float max_i_right, max_i_left = 0;
int count = 1;
int right_speed, left_speed = 200;
int ki = 1;
int sm=2;
int set=720;

void loop(){
  int kp = analogRead(6) / 24;
  int kd = analogRead(7) / 24;
  if(count % sm != 0){
    count++;
  }else{
    left = analogRead(LEFT);
    right = analogRead(RIGHT);

    last_left_error  = left_error;
    last_right_error = right_error;
    left_error  = set - left;
    right_error = set - right;
    
    // Proportional
    p_left  = kp * left_error;
    p_right = kp * right_error;
    // Derivative
    d_left = kd * (left_error - last_left_error);
    d_right = kd * (right_error - last_right_error);
    // Integral
    i_left += ki * left_error;
    if (i_left > max_i_left)  i_left = maxi_left;
    if (i_left < -max_i_left) i_left = -maxi_left;
    g_left = p_left + d_left + i_left;

    i_right += ki * right_error;
    if (i_right > max_i_right)  i_right = maxi_right;
    if (i_right < -max_i_right) i_right = -maxi_right;
    g_right = p_right + d_right + i_right;

    if (count % 100 == 0) {
      LCD.clear();
      LCD.home();
      LCD.setCursor(0,0);
      LCD.print("D: "); LCD.print(kd); LCD.print(" "); LCD.print(abs(g_right / 5000 * 255));
      LCD.setCursor(0,1);
      LCD.print("P: "); LCD.print(kp); LCD.print(" "); LCD.print(abs(g_left / 5000 * 255));
    }
    motor.speed(RIGHT, abs(g_right / 5000 * 255));
    motor.speed(LEFT, abs(g_left / 5000 * 255));
    count++;
  }
}
