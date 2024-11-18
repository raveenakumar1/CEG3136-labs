/*-------------------------------------
File: lcdDisp.c  (LCD Diplay Module)

Description: C Module that provides
             display functions on the
             LCD. It makes use of the LCD ASM 
             Module developed in assembler.
-------------------------------------*/
#include <mc9s12dg256.h>
/* Notes on mc9s12dg256.h:
   1) the type "byte" is defined as "unsigned char"
*/
#include "lcd_asm.h"

// Some Definitions
#define NUM_LINES 2
#define LINE_OFFSET 40
#define LINE_SIZE 16

// Prototypes of local functions
void padLCDString(char *, char *, byte);

/*--------------------------
Function: initLCD
Parameters: None.
Returns: nothing
Description: Initialised the LCD hardware by
             calling the assembler subroutine.
---------------------------*/

void initLCD(void)
{
  lcd_init();
}

/*--------------------------
Function: printStr

Parameters: str - pointer to string to be printed 
                  (only 16 chars are printed)
            lineno - 0 first line
                     1 second line
Returns: nothing

Description: Prints a string on the display on one of the
             two lines.  String is padded with spaces to
             erase any existing characters.
---------------------------*/
void printLCDStr(char *str, byte lineno)
{
    char newstr[LINE_SIZE+1];  // create a new string with space for null terminator
    if(lineno < 2)  // check if the line number is valid (0 or 1)
    {
       set_lcd_addr(lineno*LINE_OFFSET);  // set address for line 1 or 2 (40 for line 1)
       padLCDString(str, newstr, LINE_SIZE);  // pad the string to fit the display width
       type_lcd(newstr);  // send the padded string to the LCD
    }
    // no action if lineno is invalid
}


/*--------------------------
Function: padLCDString
Parameters: str - string to pad
            newstr - new string for padding.
            size - size of new string.
Returns: nothing
Description: Copies the string referenced by str
             to buffer referenced by newstr and
             pad with spaces to fill the buffer.
---------------------------*/
void padLCDString(char *str, char *newstr, byte size)
{
    int i=0;  // index
    
    while(i<size)
    {
      if(*str == '\0') break;
      *newstr++ = *str++;
      i++;
    }
    while(i<size)
    {
       *newstr++ = ' '; // Pad with space
       i++;
    }
    *newstr = '\0'; // terminate with nul char
}

/*--------------------------
Function: putLCDChar
Parameters: ch - character to be printed
            lineno - 0 first line
                     1 second line
            chpos - 0 to 15            
Description:  Prints the character at position 
              on the line.
---------------------------*/
void putLCDChar(char ch, byte lineno, byte chpos)
{
    byte adr;
    
    if(lineno < NUM_LINES && chpos < LINE_SIZE)  // check if line number and position are valid
    {
       adr = lineno*LINE_OFFSET + chpos;  // calculate the address based on line and position
       set_lcd_addr(adr);  // set the LCD address to the calculated one
       data8(ch);  // write the character to the LCD
    }
  
}
