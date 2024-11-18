/*-------------------------------------------------------------
 * File: armed.c
 * Description: this file contains the armed module for the
 *              alarm system project.
-----------------------------------------------------------------*/

#include "alarmExtern.h"  // definitions file

// prototypes of local functions
void systemArmed(void);

// constants
#define ARMDELAY 15000  // delay for arming and disarming
#define BEEPDELAY 1000

// armed module (armed.c)
#define CODEMSG	"Code?"
#define ARMING "*** Arming ***"
#define ARMED	"*** Armed ***"
#define DISARMING	"-- Disarming --"

// prototypes of local functions
void triggerAlarm(void);
byte checkCode(byte);
byte isCodeValid(int);

/*------------------------
 * Function: enableAlarm
 * Parameters: none
 * Returns: nothing
 * Description:
 *     gets user to enter a valid alarm code to arm
 *     the alarm system.  delays 15 seconds (to allow user
 *     to leave; the alarm can be disarmed during this period),
 *     monitors for trigger events (a, b, c, d, e).  when 'a'
 *     is detected, 15 seconds are allowed to enter an alarm code to
 *     disarm the system; otherwise the alarm is triggered. for other
 *     trigger events, the alarm is triggered instantly.  the alarm
 *     system can be turned off after being triggered with an alarm
 *     code.
 *-----------------------*/
void enableAlarm(void)
{
   byte input;  // input from user
   byte codeValid;  // valid code found
   int delayCount;  // counting 100 ms delays of segDisp
   char ch;

   // get a valid code to arm the system
   clear_lcd();
   printLCDStr(CODEMSG, 1);
   codeValid = FALSE;
   while(!codeValid) 
   {
     input = readKey();
     codeValid = checkCode(input);
   }
   printLCDStr(ARMING,1);
   
   // delay 10 seconds using segDisp 100 ms delays
   codeValid = FALSE;
   delayCount = 100;  //10sec delay
   setCharDisplay('1',2); 
   setCharDisplay('0',3);
   while(delayCount)  // loop if delayCount not zero
   {
      // update display
      segDisp();   // 100 ms display
      delayCount--;
      if(delayCount%10 == 0)
      {
         ch = 0x30+delayCount/10;
         setCharDisplay(' ',2); 
         setCharDisplay(ch,3);
      }
      // check input
      input = pollReadKey();  // check if alarm code entered to disarm
      if(isdigit(input) || input == '#') 
      {
         codeValid = checkCode(input);
         if(codeValid) delayCount = 0;  // break out of loop
      }
   }
   clearDisp();
   segDisp();
   if(!codeValid) systemArmed();
}

void systemArmed() 
{ 
   byte input;  // input from user
   byte codeValid = FALSE;  // valid code found
   int delayCount;  // counting 100 ms delays of segDisp
   char ch;

   // loop to monitor trigger events and alarm code to disable
   // codeValid is TRUE if valid alarm code entered
   if(!codeValid) printLCDStr(ARMED,1);   
   while(!codeValid)
   {
       input = pollReadKey();
       if(isdigit(input) || input == '#') codeValid = checkCode(input);
       else if(getSwStatus() & 0b00000001) // front door opened - delay before alarm
       {
           printLCDStr(DISARMING,1);
              // delay 10 seconds
           codeValid = FALSE;
           delayCount = 100;  //10 sec delay
           setCharDisplay('1',2); 
           setCharDisplay('0',3);
           while(!codeValid) 
	         {
              // update display
              segDisp();
              delayCount--;
              if(delayCount%10 == 0)
              {
                 ch = 0x30+delayCount/10;
                 setCharDisplay(' ',2); 
                 setCharDisplay(ch,3);
              }
	            // check input
              if(!delayCount)  // 0 means delay is done
              { 
                 clearDisp();
                 triggerAlarm();
                 codeValid = TRUE;
              }
              else
              {
                 input = pollReadKey();  // check if alarm code entered to disarm
                 if(isdigit(input) || input == '#') 
                     codeValid = checkCode(input); 
              }              

	         }
	         clearDisp();
	         segDisp();	         

       }
       else if((getSwStatus() & 0b11111110)) // other door/window opened
       {
           triggerAlarm();
           codeValid = TRUE;
       }
       // ignore all other input
   }
}


/*----------------------------
 * Functions: checkCode
 * Parameters: input - input character
 * Returns: TRUE - alarm code detected
 *          FALSE - alarm code not detected
 * Descriptions: creates alarm code using digits entered until
 *               4 digits are seen. after 4th digit, see if
 *               alarm code is valid using isCodeValid().
 *----------------------------*/

byte checkCode(byte input)
{
   static int mult = 1000; // current multiplier of digit
   static int alarmCode = 0;   // alarm code value
   byte retval = FALSE;

   if(isdigit(input))
   {
      alarmCode = alarmCode + (input-ASCII_CONV_NUM)*mult;
      mult = mult/10;
      if(mult == 0) 
      {
         retval = isCodeValid(alarmCode);
         alarmCode = 0;
         mult = 1000;
      }
   }
   else
   {
      alarmCode = 0;
      mult = 1000;
   }
   
   return(retval);
}

/*----------------------------
 * Functions: isCodeValid
 * Parameters: alarmCode - integer alarmCode
 * Returns: TRUE - alarm code valid
 *          FALSE - alarm code not valid
 * Descriptions: checks to see if alarm code is in the
 *               alarmCodes array.
 *----------------------------*/
byte isCodeValid(int alarmCode)
{
   int *ptr;  // pointer to alarmCodes
   byte cnt = NUMCODES;
   byte retval = FALSE;
   ptr = alarmCodes;
   do
   {
      if(*ptr++ == alarmCode)
      {
         retval = TRUE;
         break;
      }
      cnt--;
   } while(cnt != 0);
   return(retval);
}

/*----------------------------
 * Functions: triggerAlarm
 * Parameters: none
 * Returns: nothing
 * Descriptions: repeatedly sends a bel character to the
 *               terminal until a valid alarm code is entered.
 *----------------------------*/

void triggerAlarm()
{
   byte done = FALSE; // wait code is given
   volatile int doneInput;    // wait until delay done
   byte input;
   byte dispA = FALSE; // display alarm
   while(!done)
   {
       if(!dispA) 
       {
           setCharDisplay('A',0); 
           dispA = TRUE;
       }
       else
       {
           setCharDisplay(' ',0); 
           dispA = FALSE;
       } 
       doneInput = 10;
       while(doneInput) 
       {
          segDisp();  // displays characters - uses up 100ms
          input = pollReadKey();  // check if code entered to disarm
          if(isdigit(input) || input == '#') done = checkCode(input);
          doneInput--;
      }
   }
   clearDisp();
   segDisp();
} 
