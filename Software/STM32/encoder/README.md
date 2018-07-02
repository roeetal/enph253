# Rotary Encoder

http://elastic-notes.blogspot.com/p/cubemx-stm32-encoder-interface.html

- set up a timer in encoder mode
- use pull up resistors on the gpio pins
- in the generated code, remember to change the timer period from 0
- I also added internal filters on the gpio pins, would be interesting to experiment with those and see how they work.
