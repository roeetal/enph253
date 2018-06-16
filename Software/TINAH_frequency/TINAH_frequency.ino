#include <phys253.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>

volatile unsigned int count = 0;
volatile unsigned int overflow = 0;

void enableTimer()
{
    // set up timer
    // no prescaling
    TCCR0 |= (1 << CS00);
    // enable overflow interrupt
    TIMSK |= (1 << TOIE0);
    // initialize counter
    TCNT0 = 0;
}

// TIMER1 overflow interrupt service routine
// called whenever TCNT1 overflows
ISR(TIM0_OVF_vect){
    overflow++;
}

void enableExternalInterrupt(unsigned int INTX, unsigned int mode)
{
  if (INTX > 3 || mode > 3 || mode == 1) return;
  cli();
  /* Allow pin to trigger interrupts        */
  EIMSK |= (1 << INTX);
  /* Clear the interrupt configuration bits */
  EICRA &= ~(1 << (INTX*2+0));
  EICRA &= ~(1 << (INTX*2+1));
  /* Set new interrupt configuration bits   */
  EICRA |= mode << (INTX*2);
  sei();
}

void disableExternalInterrupt(unsigned int INTX)
{
  if (INTX > 3) return;
  EIMSK &= ~(1 << INTX);
}

ISR(INT0_vect) {
  count = TCNT0 + overflow * 255;
  TCNT0=0;
}

void setup(){
  #include <phys253setup.txt>
  Serial.begin(9600);
  enableExternalInterrupt(INT0, FALLING);
  enableTimer();
}


void loop(){
  Serial.println(count/16000000);
  delay(1000);
}
