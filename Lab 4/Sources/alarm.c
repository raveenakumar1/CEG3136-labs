/*-------------------------------------------------------------
 * File:  alarm.c  - Alarm System Module
 * Description: The design of the assembler code is achieved using
 *              a C program.  This program is designed to create a
 *              Alarm System on the Dragon 12 Trainer card (i.e.
 *              the HCS12 Microcontroller).  The main objective of this
 *              program is to explore programming of the HC12 CPU.
-----------------------------------------------------------------*/
#include "main_asm.h"
#include "alarm.h"  // Definitions file
#include "string.h"

/*--------------Global Data Structures--------------------------*/
// The following data structures need not be located in RAM - They are 
// readonly

// Alarm codes - needs to be placed in EEPROM
#pragma DATA_SEG EEPROM_DATA
int alarmCodes[NUMCODES] = { 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
#pragma DATA_SEG DEFAULT 
int displayTempFlag;  // TRUE - display temp, FALSE - Do not display temp.

// Although these are defines, the strings must be stored somewhere
// in memory.  This means that the symbolic constants (such as WELCOME)
// represent the address to the strings. Need to store them in the
// EEPROM (read-only) memory
#define MENU1 "CEG Alarm System"
#define MENU2 "c-Config a-Arm"
// For configuring Channel 6 to display temperature
#define TEN_MS 7500  // 7500 * 1.3333 micro-sec = 10 ms


// Prototypes
void initMain(void);
void displayTemp(int); 


/*------------------------------------
 * Function: main
 * Description: The main routine for running the game.
 *              Initializes things (via initgb) and then
 *              loops forever playing games.
 * --------------------------------*/
void main()
{
   byte select;
   PLL_init();  // initialize phase-locked loop (PLL) for clock
   initMain();  // initialize main settings

   // main loop
   for(;;)  // loop forever
   {
      printLCDStr(MENU1, 0);  // display first menu on LCD
      printLCDStr(MENU2, 1);  // display second menu on LCD
      select = pollReadKey(); // read user input from the keypad

      if(select == 'c') 
         configCodes();  // if 'c' pressed, configure alarm codes
      else if(select == 'a') 
         enableAlarm();  // if 'a' pressed, enable alarm
      else 
         ;  // do nothing if no valid key is pressed
   }
}


/*------------------------------------
 * Function: intMain
 * Description: Main initialisation to 
 *              initialise modules and 
 *              the Alarm Module.
 * --------------------------------*/
void initMain()
{
   // initialisation of various components
   initCodes();  // initialize alarm codes
   initKeyPad();  // initialize keypad
   initSwitches();  // initialize switches
   initDisp();  // initialize display

   // setup the timer
   TSCR1 = 0b10010000;  // enable the timer and enable fast clear
   TSCR2 = 0b00000101;  // setup prescaler to 32, for 1 1/3 micro-sec tick

   initSiren();  // initialize siren
   initDelay();  // initialize delay module
   asm cli;  // clear interrupt flag
   initLCD();   // need to initialize with interrupts running since delay module is used
}

