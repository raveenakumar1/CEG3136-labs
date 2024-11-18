/*-----------------------------------------------------------
    File: keyPad.c
    Description: Module for reading the keypad using interrupts
                 and timer channel 4.
-------------------------------------------------------------*/

#include "mc9s12dg256.h"
#include "keyPad.h"
#define BIT4 0b00010000;

#define TENMSEC 7500  // 10 ms = 7500 x 1 1/3 micro-second

// Global variables
volatile byte keyCode;

// Local Function Prototypes
char getAscii(byte);
byte getKCode(void);

/*---------------------------------------------
Function: initKeyPad
Description: initializes hardware for the 
             KeyPad Module.
-----------------------------------------------*/
void initKeyPad(void) 
{
  // set up port a register
  DDRA = 0xF0;  // configure lower 4 bits of port a as outputs
  PORTA = 0x00;  // initialize all outputs to 0 (should read 0xF0)
  PUCR |= 0x01;  // enable pull-up resistors on port a
  
  // set up timer channel for interrupt generation
  // assume timer is already enabled elsewhere with 1 1/3 microsec ticks
  // used for controlling displays
  TIOS |= BIT4;  // set output compare mode for timer channel 4 (TC4)
  TIE |= BIT4;   // enable interrupt for TC4
  
  TC4 = TCNT + TENMSEC;  // set TC4 to trigger timeout after 10 ms
  keyCode = NOKEY;  // initialize keyCode as no key pressed

/*-------------------------------------------------
Interrupt: readKey
Description: Waits for a key and returns its ASCII
             equivalent.
---------------------------------------------------*/
char readKey() 
{
    char ch;
    while(keyCode == NOKEY) /* wait until a key is pressed */;  
    ch = getAscii(keyCode);  // convert keyCode to ASCII character
    keyCode = NOKEY;  // reset keyCode to no key pressed
    return(ch);  // return the ASCII character of the pressed key
}

/*-------------------------------------------------
Interrupt: pollReadKey
Description: Checks for a key and if present returns its ASCII
             equivalent; otherwise returns NOKEY.
---------------------------------------------------*/
char pollReadKey() 
{
    char ch;
    if(keyCode == NOKEY) ch = NOKEY;  // return no key if no key is pressed
    else
    {  
        ch = getAscii(keyCode);  // convert keyCode to ASCII character
        keyCode = NOKEY;  // reset keyCode to no key pressed
    }
    return(ch);  // return the ASCII character of the pressed key or NOKEY
}


/*-------------------------------------------------
Interrupt: key_isr
Description: Display interrupt service routine
             that checks keypad every 10 ms.
---------------------------------------------------*/
// State values
#define WAITING_FOR_KEY 0
#define DEB_KEYPRESS    1 
#define WAITING_FOR_REL 2
#define DEB_REL         3 

void interrupt VectorNumber_Vtimch4 key_isr(void)
{
  static byte state = WAITING_FOR_KEY;  // state of keypad check
  static byte code;
  
  switch(state) 
  {
    case WAITING_FOR_KEY:
      code = PORTA;
      if(code != 0x0F) state = DEB_KEYPRESS;
      break;
    case DEB_KEYPRESS:
      if(PORTA != code) state = WAITING_FOR_KEY;
      else 
      {
         code = getKCode();
         state = WAITING_FOR_REL;       
      }
      break;
    case WAITING_FOR_REL:
      if(PORTA == 0x0F) state = DEB_REL;
      break;
    case DEB_REL:
      if(PORTA != 0x0F) state = WAITING_FOR_REL;
      else 
      {
          keyCode = code;  // save ASCII code value
          state = WAITING_FOR_KEY;
      }
      break;
  }
	// Set up next interrupt (also clears the interrupt)
	TC4 = TC4 + TENMSEC;
}

/*-------------------------------------------------
Interrupt: getKCode()
Description: Gets a code from PORTA that corresponds
             to a keypress.
---------------------------------------------------*/
#define ROW1 0b11101111
#define ROW2 0b11011111
#define ROW3 0b10111111
#define ROW4 0b01111111
byte getKCode()
{
  volatile byte code;
 
  PORTA = ROW1;
  if( PORTA == ROW1)
  {
     PORTA = ROW2;
     if( PORTA == ROW2)
     {
        PORTA = ROW3;
        if( PORTA == ROW3) 
        {
           PORTA = ROW4;  // Assume bit 3 produces code
        }
     }
  } 
  code = PORTA;
  PORTA = 0x00; // set all output pins to low
  return(code);
}
/*-------------------------------------------------
Interrupt: getAscii()
Description: Gets a code from PORTA that corresponds
             to a keypress.
---------------------------------------------------*/
byte kCodes[] =  { 0b11101110, 0b11101101, 0b11101011, 0b11100111, 
                   0b11011110, 0b11011101, 0b11011011, 0b11010111, 
                   0b10111110, 0b10111101, 0b10111011, 0b10110111, 
                   0b01111110, 0b01111101, 0b01111011, 0b01110111, 
                   BADCODE };
byte aCodes[] =  { '1',        '2',        '3',        'a',    
                   '4',        '5',        '6',        'b',
                   '7',        '8',        '9',        'c',
                   '*',        '0',        '#',        'd', 
                   BADCODE };


char getAscii(byte cd)
{
    int i;
    char ch = BADCODE;
    for(i=0; kCodes[i] != BADCODE ; i++) 
    {
       if(kCodes[i] == cd) 
       {
          ch = aCodes[i];
          break;
       }
    }
    return(ch);
}


