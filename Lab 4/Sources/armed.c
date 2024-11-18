/*-------------------------------------------------------------
 * File: armed.c
 * Description: This file contains the Amred module for the
 *              Alarm System Project.
-----------------------------------------------------------------*/

#include "alarmExtern.h"  // Definitions file
#include "stdio.h"

// Constants
#define ARMDELAY 10000  // delay for arming and disarming = 10 seconds
#define BEEPDELAY 1000

// Armed Module (armed.c)
#define CODEMSG	"Code?"
#define ARMING "*** Arming ***"
#define ARMED	"*** Armed ***"
#define DISARMING	"-- Disarming --"

// Prototypes of local functions
void systemArmed(void);
void triggerAlarm(void);
byte checkCode(byte);
byte isCodeValid(int);
void displayNum(int);

/*------------------------
 * Function: enableAlarm
 * Parameters: none
 * Returns: nothing
 * Description:
 *     Gets user to enter a valid alarm code to arm
 *     the alarm system.  Delays 15 seconds (to allow user
 *     to leave; the alarm can be disarmed during this period),
 *     monitors for trigger events (a, b, c, d, e).  When 'a'
 *     is detected, 15 seconds are allowed to enter an alarm code to
 *     disarm the system; otherwise the alarm is triggered. For other
 *     trigger events, the alarm is triggered instantly.  The alarm
 *     system can be turned off after being triggered with an alarm
 *     code.
 *-----------------------*/
void enableAlarm(void)
{
   byte input;  // user input
   byte codeValid;  // flag to check if valid code entered
   volatile int delayCount;  // counter for 10 sec delays during segDisp

   // prompt user for valid code to arm the system
   printLCDStr(CODEMSG, 1);
   codeValid = FALSE;
   while(!codeValid) 
   {
     input = readKey();  // read user input
     codeValid = checkCode(input);  // check if input code is valid
   }
   printLCDStr(ARMING,1);
   // stop displaying temperature
   displayTempFlag = FALSE;
   clearDisp();
   // delay 10 seconds using segDisp for 100 ms intervals
   codeValid = FALSE;
   setCounter(&delayCount);  // enable counter
   delayCount = ARMDELAY;  // set delay to 10 seconds
   while(delayCount>0)  // loop until delayCount reaches 0
   {
      // update display every second
      if(delayCount%1000 == 0) // every 1000 ms (1 sec)
         displayNum(delayCount/1000);
      // check user input
      input = pollReadKey();  // read key input
      if(isdigit(input) || input == '#') 
      {
         codeValid = checkCode(input);  // check code for validity
         if(codeValid) delayCount = 0;  // exit loop if valid code entered
      }
   }
   setCounter(NULL);  // disable counter
   clearDisp();
   // resume displaying temperature
   displayTempFlag = TRUE;
   if(!codeValid) systemArmed();  // if no valid code, arm system
}
/*----------------------------
 * Function: systemArmed
 * Parameters: None
 * Returns: None
 * Description: Monitors alarm status after system is armed. 
 *              Checks for valid alarm code input to disarm 
 *              the system. If the front door is opened, the 
 *              system waits before triggering the alarm, 
 *              and checks if any other door/window is opened.
 *              If any door/window opens, the alarm is triggered.
 *              The function manages a 10-second delay and handles 
 *              disarming when the correct code is entered.
 *----------------------------*/

void systemArmed() 
{ 
   byte input;  // user input
   byte codeValid = FALSE;  // flag to check if valid code entered
   volatile int delayCount;  // counter for 100 ms delays during segDisp

   // loop to monitor triggers and alarm code to disable alarm
   // codeValid is TRUE if valid alarm code entered
   if(!codeValid) printLCDStr(ARMED,1);   
   while(!codeValid)
   {
       input = pollReadKey();  // read user input
       if(isdigit(input) || input == '#') 
           codeValid = checkCode(input);  // check if input code is valid
       else if(getSwStatus() & 0b00000001) // front door opened - delay before alarm
       {
           triggerAlarm();   // trigger alarm
           printLCDStr(DISARMING,1);
           // stop displaying temperature
           displayTempFlag = FALSE;
           clearDisp();
           // delay 10 seconds before triggering alarm
           codeValid = FALSE;
           setCounter(&delayCount);   // enable counter
           delayCount = ARMDELAY;  // 10 sec delay
           while(!codeValid) 
           {
              // update display every second
              if(delayCount%1000 == 0)                 
                 displayNum(delayCount/1000);
              // check for input
              if(delayCount <= 0)  // check if delay is finished
              { 
                 clearDisp();
                 displayTempFlag = TRUE;
                 triggerAlarm();  // trigger alarm once delay is over
                 codeValid = TRUE;  // exit loop after alarm is triggered
              }
              else
              {
                 input = pollReadKey();  // check if alarm code entered to disarm
                 if(isdigit(input) || input == '#') 
                     codeValid = checkCode(input); 
              }              
           }
           setCounter(NULL);   // disable counter
           clearDisp();
           // start displaying temperature again
           displayTempFlag = TRUE;
       }
       else if((getSwStatus() & 0b11111110)) // other door/window opened
       {
           triggerAlarm();  // trigger alarm if any door/window is opened
           codeValid = TRUE;  // exit loop after triggering alarm
       }
       // ignore other input
   }
}



/*----------------------------
 * Functions: checkCode
 * Parameters: input - input character
 * Returns: TRUE - alarm code detected
 *          FALSE - alarm code not detected
 * Descriptions: Creates alarm code using digits entered until
 *               4 digits are seen. After 4th digit, see if
 *               alarm code is valid using isCodeValid().
 *----------------------------*/
byte checkCode(byte input)
{
   static int mult = 1000; // multiplier for the digit's place value
   static int alarmCode = 0;   // stores the current alarm code value
   byte retval = FALSE;  // return value indicating if code is valid

   // check if the input is a digit
   if(isdigit(input))
   {
      alarmCode = alarmCode + (input-ASCII_CONV_NUM)*mult;  // update alarm code
      mult = mult/10;  // move to the next place value (e.g., from thousands to hundreds)
      if(mult == 0)  // check if all 4 digits have been entered
      {
         retval = isCodeValid(alarmCode);  // validate the entered code
         alarmCode = 0;  // reset the alarm code
         mult = 1000;  // reset the multiplier
      }
   }
   else
   {
      alarmCode = 0;  // reset alarm code if input is not a digit
      mult = 1000;  // reset the multiplier
   }
   
   return(retval);  // return if the code is valid or not
}

/*----------------------------
 * Functions: isCodeValid
 * Parameters: alarmCode - integer alarmCode
 * Returns: TRUE - alarm code valid
 *          FALSE - alarm code not valid
 * Descriptions: Checks to see if alarm code is in the
 *               alarmCodes array.
 *----------------------------*/
byte isCodeValid(int alarmCode)
{
   int *ptr;  // pointer to the array of valid alarm codes
   byte cnt = NUMCODES;  // number of valid codes to check
   byte retval = FALSE;  // return value, default is invalid code
   ptr = alarmCodes;  // initialize pointer to the first valid alarm code

   // loop through the alarm codes
   do
   {
      // check if the current code matches the entered alarm code
      if(*ptr++ == alarmCode)
      {
         retval = TRUE;  // code is valid
         break;  // exit loop if valid code found
      }
      cnt--;  // decrement the counter
   } while(cnt != 0);  // continue until all codes are checked

   return(retval);  // return if the code is valid or not
}

/*----------------------------
 * Functions: triggerAlarm
 * Parameters: none
 * Returns: nothing
 * Descriptions: Repeatedly sends a bel character to the
 *               terminal until a valid alarm code is entered.
 *----------------------------*/

void triggerAlarm()
{
   byte done = FALSE; // flag to indicate if the alarm code is entered
   byte input;
   turnOnSiren();  // activate the siren

   // loop until a valid code is entered
   while(!done)
   {
      input = pollReadKey();  // check for user input to disarm; 1ms delay
      if(isdigit(input) || input == '#') 
             done = checkCode(input);  // validate the entered code
   }

   turnOffSiren();  // deactivate the siren once the code is valid
}


/*-------------------------------------------
Function: displayNum
Paramter: num - number to display, must be < 99
Description: Displays a 2 digit number on 7-segment displays 2 and 3.
---------------------------------------------*/
void displayNum(int num) 
{
   char dig1, dig2, ch;

   if(num > 99) return;  // don't display anything if the number is too large
   dig2 = num%10;   // extract the units digit
   dig1 = num/10;   // extract the tens digit

   ch = 0x30+dig1;
   if(dig1 == 0) 
      setCharDisplay(' ',2);  // don't display tens place if it's zero
   else 
      setCharDisplay(ch,2);  // display tens digit

   ch = 0x30+dig2; 
   setCharDisplay(ch,3);  // display units digit
}

