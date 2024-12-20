/*--------------------------------------------
File: SegDisp.c
Description:  Segment Display Module
              Uses Timer Channel 1
---------------------------------------------*/

#include "mc9s12dg256.h"
#include "SegDisp.h"

#define NUMDISPS 4  // number of displays
#define SPACE ' '   // The space character
#define BLANK 0xFF  // disables displays
#define NUMFLASH 10 // Number of flashes
#define DISP_TIMEOUT 3750	// For 50 ms delay (1 1/3 microSec/tick).
#define HALFSEC 500  // 500 ms = 1/2 sec


// Global Variables
byte codes[NUMDISPS];  // ASCII codes for each display

struct ascii_to_code 
{
   byte ascii;
   byte code;  
}; 

#define NUMCHS  18 // number of entries in dispTbl
struct ascii_to_code dispTbl[NUMCHS] = 
{          //gfedcba  <- display segments
    { '0',0b00111111 },
    { '1',0b00000110 },
    { '2',0b01011011 },
    { '3',0b01001111 },
    { '4',0b01100110 },
    { '5',0b01101101 },
    { '6',0b01111101 },
    { '7',0b00000111 },
    { '8',0b01111111 },
    { '9',0b01101111 },
    { '*',0b01000110 },
    { '#',0b01110000 },
    { 'a',0b01110111 },
    { 'b',0b01111100 },
    { 'c',0b00111001 },
    { 'd',0b01011110 },
    { 'e',0b01111001 },
    { ' ',0b00000000 }  // space
};

byte enableCodes[NUMDISPS]= {  
     0b00001110,	// display 0
     0b00001101,	// display 1
     0b00001011,	// display 2
     0b00000111	  // display 3
};

// Prototypes for internal functions

byte getCode(byte ch);

/*---------------------------------------------
Function: initDisp
Description: initializes hardware for the 
             7-segment displays.
-----------------------------------------------*/
void initDisp(void) 
{
  // configure port b and port p to control display
  DDRB = 0xFF;  // set all bits of port b as output
  DDRP |= 0x0F;  // set bits 0 to 3 of port p as output
  PTP |= 0x0F;   // disable all displays initially
  clearDisp();   // clear the displays at the start
  
  // set up timer to generate interrupts for display control
  // assume timer is already enabled elsewhere with 1 1/3 microsecond ticks
  TIOS |= 0b00000010; // set output compare on timer channel 1
  TIE |= 0b00000010;  // enable interrupt on timer channel 1
  
  TC1 = TCNT + DISP_TIMEOUT;  // set timer channel 1 to trigger after a timeout
}


/*---------------------------------------------
Function: clearDisp
Description: Clears all displays.
-----------------------------------------------*/
void clearDisp(void) 
{
   int i;
   // loop through all displays and set their codes to 0
   for(i=0 ; i<NUMDISPS ; i++)
      codes[i] = 0;
}


/*---------------------------------------------
Function: setCharDisplay
Description: Receives an ASCII character (ch)
             and translates
             it to the corresponding code to 
             display on 7-segment display.  Code
             is stored in appropriate element of
             codes for identified display (dispNum).
-----------------------------------------------*/
void setCharDisplay(char ch, byte dispNum) 
{
  byte code;

  // get the display code for the character
  code = getCode(ch);

  // set the display code for the specified display, preserving the decimal point (bit 7)
  codes[dispNum] = code | (codes[dispNum] & 0x80);
}

/*---------------------------------------------
Function: getCode
Description: Translates an ASCII character code
             (ch) to a 7-segment display code. 
             Returns 0 (blank) if character is
             not in the table;
-----------------------------------------------*/
byte getCode(byte ch) 
{
   byte code = 0;
   byte i;

   // search through the display table to find the corresponding code for the character
   for(i = 0; i < NUMCHS && code == 0; i++)
     if(ch == dispTbl[i].ascii) 
        // set the code when a match is found
        code = dispTbl[i].code;

   return(code);  
}


/*---------------------------------------------
Function: turnOnDP
Description: Turns on the decimal point of 2nd
             display from the left.
-----------------------------------------------*/
void turnOnDP(int dNum) 
{
    codes[dNum] = codes[dNum] | 0x80;  // sets bit 7  
}

/*---------------------------------------------
Function: turnOffDP
Description: Turns off the decimal point of 2nd
             display from the left.
-----------------------------------------------*/
void turnOffDP(int dNum) 
{
    codes[dNum] = codes[dNum] & 0x7f;  // clears bit 7  
}


/*-------------------------------------------------
Interrupt: disp_isr
Description: Display interrupt service routine that
             to update displays every 50 ms.
---------------------------------------------------*/
void interrupt VectorNumber_Vtimch1 disp_isr(void)
{
  static byte dNum = 0;  // preserve between invocations
  byte enable;
  
  PORTB = codes[dNum];
  enable = PTP;  // get current values
  enable &= 0xF0; // erase lower four bits
  PTP = enable | enableCodes[dNum]; // set lower for bits
  dNum++;
  dNum = dNum%NUMDISPS;
	// Set up next interrupt (also clears the interrupt)
	TC1 = TC1 + DISP_TIMEOUT;
}



