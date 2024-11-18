/*------------------------------------------------
File: siren.c

Description: The siren module.
-------------------------------------------------*/
#include "mc9s12dg256.h"  // include the header for the microcontroller

// definitions for the high and low durations of the siren signal
#define HIGH_MS 300   // 300 * 1 1/3 micro-sec = 0.400 ms 
#define LOW_MS 600    // 600 * 1 1/3 micro-sec = 0.800 ms

// prototypes of local functions
void interrupt VectorNumber_Vtimch5 sirenISR(void);

/*------------------------------------------------
Function: initSiren

Description: Initializes the siren by setting up the output-compare mode for TC5.
-------------------------------------------------*/
void initSiren()
{
   TIOS |= 0b00100000;  // set TC5 to output-compare mode
}

/*------------------------------------------------
Function: turnOnSiren

Description: Turns on the siren by setting pin 5 high at an output-compare event and enabling interrupts.
-------------------------------------------------*/
#define HIGH 1
#define LOW 0
int levelTC5;  // stores the current level on TC5
void turnOnSiren()
{
   TCTL1 |= 0b00001100;  // set pin 5 to high on output-compare event 
   CFORC = 0b00100000;   // force an event on TC5 (set pin 5 high)
   levelTC5 = HIGH;      // set current level to high
   TCTL1 &= 0b11110111;  // toggle pin state at output-compare event
   TC5 = TCNT + HIGH_MS; // set TC5 to trigger after HIGH_MS time
   TIE |= 0b00100000;    // enable interrupt for TC5
}

/*------------------------------------------------
Function: turnOffSiren

Description: Turns off the siren by disabling the interrupt and setting pin 5 low at an output-compare event.
-------------------------------------------------*/
void turnOffSiren()
{
   TIE &= 0b11011111;  // disable interrupt for TC5
   TCTL1 |= 0b00001000; // set pin 5 to low at output-compare event 
   TCTL1 &= 0b11111011; // ensure pin 5 is set low
   CFORC = 0b00100000;  // force an event on TC5 (set pin 5 low)
}

/*------------------------------------------------
Function: sirenISR

Description: Interrupt service routine for TC5. Toggles the siren state between high and low.
-------------------------------------------------*/
void interrupt VectorNumber_Vtimch5 sirenISR()
{
   if(levelTC5 == HIGH)  // if the current level is high
   {
      TC5 += LOW_MS;    // set the next event for LOW_MS
      levelTC5 = LOW;    // change state to low
   }
   else  // if the current level is low
   {
      TC5 += HIGH_MS;   // set the next event for HIGH_MS
      levelTC5 = HIGH;   // change state to high
   }
}
