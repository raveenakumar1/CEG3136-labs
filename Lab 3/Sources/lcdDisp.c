/*-------------------------------------
File: lcdDisp.c  (LCD Diplay Module)

Description: C Module that provides
             display functions on the
             LCD. It makes use of the LCD ASM 
             Module developed in assembler.
-------------------------------------*/
#include "alarm.h"
#include "lcd_asm.h"

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
    byte i = 0;  // variable to store LCD address

    // check which line to print on (line 1 or line 2)
    if(lineno == 0x01){  
        i = 0x40;  // set the address to the start of the second line (0x40)
    }
    
    set_lcd_addr(i);  // set the LCD address to the correct line
    type_lcd(str);  // print the string to the LCD

}
