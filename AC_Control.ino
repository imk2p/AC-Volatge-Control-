// AC Control V1.1
//
// This Arduino sketch is for use with the heater 
// control circuit board which includes a zero 
// crossing detect function and an opto-isolated TRIAC.
//
// AC Phase control is accomplished using the internal 
// hardware timer1 in the Arduino
//
// Timing Sequence
// * timer is set up but disabled
// * zero crossing detected on pin 2
// * timer starts counting from zero
// * comparator set to "delay to on" value
// * counter reaches comparator value
// * comparator ISR turns on TRIAC gate
// * counter set to overflow - pulse width
// * counter reaches overflow
// * overflow ISR turns off TRIAC gate
// * TRIAC stops conducting at next zero cross


// The hardware timer runs at 16MHz. Using a
// divide by 256 on the counter each count is 
// 16 microseconds.  1/2 wave of a 50Hz AC signal
// is about 625 counts (10,000 microseconds).


#include <avr/io.h>
#include <avr/interrupt.h>

#include <IRremote.h>

int RECV_PIN = A0;

IRrecv irrecv(RECV_PIN);

decode_results results;

int led = 13;

#define DETECT 2  //zero cross detect
#define GATE 4    //TRIAC gate
#define PULSE 4   //trigger pulse width (counts)
#define MAX_VAL 490
int start_cycle = 0;
int i=MAX_VAL;

void setup(){

  // set up pins
  pinMode(DETECT, INPUT);     //zero cross detect
  digitalWrite(DETECT, HIGH); //enable pull-up resistor
  pinMode(GATE, OUTPUT);      //TRIAC gate control

  pinMode(led, OUTPUT);
  irrecv.enableIRIn(); // Start the receiver
  // set up Timer1 
  //(see ATMEGA 328 data sheet pg 134 for more details)
  OCR1A = 100;      //initialize the comparator
  TIMSK1 = 0x03;    //enable comparator A and overflow interrupts
  TCCR1A = 0x00;    //timer control registers set for
  TCCR1B = 0x00;    //normal operation, timer disabled


  // set up zero crossing interrupt
  attachInterrupt(0,zeroCrossingInterrupt, RISING);    
    //IRQ0 is pin 2. Call zeroCrossingInterrupt 
    //on rising signal

}  

//Interrupt Service Routines

void zeroCrossingInterrupt(){ //zero cross detect   
  TCCR1B=0x04; //start timer with divide by 256 input
  TCNT1 = 0;   //reset timer - count from zero
}

ISR(TIMER1_COMPA_vect){ //comparator match
  digitalWrite(GATE,HIGH);  //set TRIAC gate to high
  TCNT1 = 65536-PULSE;      //trigger pulse width
}

ISR(TIMER1_OVF_vect){ //timer1 overflow
  digitalWrite(GATE,LOW); //turn off TRIAC gate
  TCCR1B = 0x00;          //disable timer stopd unintended triggers
}

void loop(){ // sample code to exercise the circuit
if (irrecv.decode(&results)) {

//Serial.println(results.value, HEX);
if(results.value == 0x1FE50AF)
{
  digitalWrite(led, HIGH);
//  i -= 5;
  if(i>200)i -= 25;
  start_cycle = 1;
}
else if(results.value == 0x1FED827)
{
  digitalWrite(led, LOW);
  if(i<MAX_VAL)i += 25;
  start_cycle = 1;
  if(i>=MAX_VAL)
  {
    i=MAX_VAL;
    start_cycle = 0;
    OCR1A = 0;
  }
}
irrecv.resume(); // Receive the next value }

}
//i--;
if(start_cycle == 1)
{
OCR1A = i;     //set the compare register brightness desired.
delay(17);                             
}

}
