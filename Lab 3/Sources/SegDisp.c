/*--------------------------------------------
File: SegDisp.c
Description:  Segment Display Module
---------------------------------------------*/

#include <stdtypes.h>
#include "mc9s12dg256.h"
#include "SegDisp.h"
#include "Delay_asm.h"

#define LCD_DISP_ROW 2
#define LCD_DISP_COL 16
#define ASCII_SPACE 32

#define ZERO 0x3F
#define ONE  0x06
#define TWO 0x5b
#define THREE 0x4F
#define FOUR 0x6D
#define FIVE  0x66
#define SIX 0x7D
#define SEVEN 0x07
#define EIGHT 0xFF
#define NINE 0x6F

char arr[4]  = {0x00, 0x00, 0x00, 0x00};

/*---------------------------------------------
Function: initDisp
Description: initializes hardware for the 
             7-segment displays.
-----------------------------------------------*/
void initDisp(void) 
{
  DDRB = 0xFF;  // set all pins of port B as output (0xFF = 11111111 in binary)
  DDRP = 0x0F;  // set the lower 4 pins of port P as output (0x0F = 00001111 in binary)
  clearDisp();  // call the function to clear the display
}


/*---------------------------------------------
Function: clearDisp
Description: Clears all displays.
-----------------------------------------------*/
void clearDisp(void) 
{
    int i = 0;

    PORTB = 0x00;  // set all pins of port B to low, turning off all display outputs
    PTP = 0x11;    // set specific pins of port P to a known state (e.g., turn off certain displays or reset pins)

    // loop through an array and set all elements to 0x00
    while(i < 4)
    {
        arr[i] = 0x00;  // clear each element of the array
        i++;             // increment the index to move to the next array element
    } 
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
    // Function to set the 7-segment display pattern for a given character on a specific display

    switch(ch) {
        case '0' :
            arr[(int)dispNum] = 0x3F;  // 7-segment code for displaying '0'
            break;
        case '1' :
            arr[(int)dispNum] = 0x06;  // 7-segment code for displaying '1'
            break;
        case '2' :
            arr[(int)dispNum] = 0x5B;  // 7-segment code for displaying '2'
            break;
        case '3' :
            arr[(int)dispNum] = 0x4F;  // 7-segment code for displaying '3'
            break;
        case '4' :
            arr[(int)dispNum] = 0x66;  // 7-segment code for displaying '4'
            break;
        case '5' :
            arr[(int)dispNum] = 0x6D;  // 7-segment code for displaying '5'
            break;
        case '6' :
            arr[(int)dispNum] = 0x7D;  // 7-segment code for displaying '6'
            break;
        case '7' :
            arr[(int)dispNum] = 0x07;  // 7-segment code for displaying '7'
            break;  
        case '8' :
            arr[(int)dispNum] = 0x7F;  // 7-segment code for displaying '8'
            break;
        case '9' :
            arr[(int)dispNum] = 0x67;  // 7-segment code for displaying '9'
            break;
        default:
            arr[(int)dispNum] = 0x00;  // default case clears the display (turns it off)
            break;
    }
}


/*---------------------------------------------
Function: segDisp
Description: Displays the codes in the code display table 
             (contains four character codes) on the 4 displays 
             for a period of 100 milliseconds by displaying 
             the characters on the displays for 5 millisecond 
             periods.
-----------------------------------------------*/
void segDisp(void) 
{
    int x;
    for(x = 0; x < 5; x++) {
        // display the value for the first 7-segment
        PORTB = arr[0];        // set the value for the first display
        PTP = 0xE;             // activate the first display
        delayMs(5);            // wait for 5 ms

        // display the value for the second 7-segment
        PORTB = arr[1];        // set the value for the second display
        PTP = 0xD;             // activate the second display
        delayMs(5);            // wait for 5 ms

        // display the value for the third 7-segment
        PORTB = arr[2];        // set the value for the third display
        PTP = 0xB;             // activate the third display
        delayMs(5);            // wait for 5 ms

        // display the value for the fourth 7-segment
        PORTB = arr[3];        // set the value for the fourth display
        PTP = 0x7;             // activate the fourth display
        delayMs(5);            // wait for 5 ms
    }
}
