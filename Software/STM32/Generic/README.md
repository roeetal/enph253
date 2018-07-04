## Useful Lynx

- [STM32 Timer Functions](http://www.dmi.unict.it/~santoro/teaching/lap1/slides_pic/TimerSTM32.pdf)
- [Other Useful Information](https://en.wikipedia.org/wiki/Lynx)

# Sonar Data and Information

__If this is unreadable, let me know. I'll type it up.__

![Image](sonar.JPG)

## Distance Equation

Sonar Distance = (Prescaler \* Sensor Value \* 340 m/s) / (72 MHz \* 2)
__note: I am not sure if the frequency of the timer is 72 MHz or 84 MHz - this is something to figure out__

## SONAR TODO

- Refine TIM2 Prescaler value to get best values
- Speed it up