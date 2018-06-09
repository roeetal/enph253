#include <phys253.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>

volatile unsigned int count = 0;
volatile unsigned int overflow = 0;

void enableExternalInterrupt()
{
    cli();
    // set up timer
    // no input capture noise canceller
    // input capture edge select: rising
    // no prescaling
    TCCR1B |= (0 << CS11);
  
    // initialize counter
    TCNT1 = 0;
  
    // enable overflow interrupt
    // enable input capture interrupt
    TIMSK |= (1 << TICIE1) | (1 << TOIE1);
  
    // enable global interrupts
    sei();
}

// Input capture interrupt
// count = number of clock ticks per period.
// there are 16 bits, 65535 periods per overflow
// each clock cycle ~ 16 MHz
ISR(TIMER1_CAPT_vect){
    count = ICR1;
    TCNT1 = 0;
    count += overflow*65535;
    overflow = 0;
}

// TIMER1 overflow interrupt service routine
// called whenever TCNT1 overflows
ISR(TIMER1_OVF_vect){
    overflow++;
}

void setup(){
  #include <phys253setup.txt>
  Serial.begin(9600);
  enableExternalInterrupt();
}


void loop(){
  Serial.println(count/16000000);
  delay(1000);
}
