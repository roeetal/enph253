# Overview #

# darkstuff #

# wiringPi #
This library lets us use the GPIO pins on the raspberry pi from the main c program. The pins differ from standard raspberry pi pinout diagrams. The pinout used by this library can be found at https://projects.drogon.net/raspberry-pi/wiringpi/pins/ 

When beginning, the pin numbering must be setup by calling:
 ```
 wiringPiSetup();
 ```

Pins are configured using:
 ```
 pinMode([pin],[mode]);
 ```

Write to pins by calling:
 ```
 digitalWrite([pin],[HIGH\LOW]);
 ```
