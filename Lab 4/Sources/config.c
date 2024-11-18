/*-------------------------------------------------------------
 * File:  config.c
 * Description: This file contains the Configuration module for the 
 *              Alarm System Project project.
-----------------------------------------------------------------*/
#include "alarmExtern.h"  // Definitions file

// Some definitions
#define MSTCDMSG "Master code?"
#define CONFIGMSG "a:mstr 1-4:other"
#define CERRMSG "Bad entry"
#define GET_CODE_MSG "Code or 'd'"
#define ERR_MST_MSG "Cannot disable"

// Prototypes of local functions
byte enterMstCode(void);
void setcode(byte);
int writeToEE(int, int);
int initEE(int, int);

/*---------------------
 * Function: configCodes
 * Parameters: none
 * Returns: nothing
 * Description: Gets user to select alarm code to update/disable. Call
 *              setcode to update the alarm code.
 * ---------------------*/
void configCodes()
{
   byte ix;  // index for accessing alarm code array
   byte input; // user input
   byte flag;  // flag to control loop

   if(enterMstCode())  // check if the master code is correct
     do
     {
        printLCDStr(CONFIGMSG,1);  // display configuration message
        input = readKey();  // get user input
        flag = TRUE;  // assume valid input

        if(input == 'a') 
           setcode(0);  // set code for option 'a'
        else if(input >= '1' && input <= '4')  // check if input is between '1' and '4'
        {
          ix = input - ASCII_CONV_NUM;  // convert input to index
          setcode(ix);  // set the code based on the input
        }
        else 
        {
           printLCDStr(CERRMSG,1);  // display error message for invalid input
           delayms(1000);  // wait for 1 second
           flag = FALSE;  // invalid input, loop again
        }
     } while(!flag);  // repeat until valid input
}


/*--------------------------------
 * Function: enterMstCode
 * Parameters: none
 * Returns: TRUE - valid code entered, FALSE otherwise.
 * Description: Prompts user for the 4 digit master alarm code.
 *-------------------------------*/
byte enterMstCode(void)
{
   byte retval = FALSE;  // return value to indicate if the master code is correct
   byte i;  // loop index
   byte input;  // user input
   int alarmCode = 0;  // variable to store the entered alarm code
   int mult = 1000;  // multiplier for each digit of the code

   printLCDStr(MSTCDMSG,1);  // display message asking for master code
   for(i = 0; i < 4; i++)  // loop for 4 digits of the master code
   {
      input = readKey();  // read key input
      if(!isdigit(input)) break;  // break loop if input is not a digit
      else
      {
        alarmCode = alarmCode + mult * (input - ASCII_CONV_NUM);  // build the alarm code
        mult = mult / 10;  // reduce the multiplier for the next digit
        if(mult == 0)  // check if all digits are entered
        {
          if(alarmCode == alarmCodes[0]) retval = TRUE;  // check if entered code matches the stored code
        }
      }
   }
   return(retval);  // return TRUE if the code matched, otherwise FALSE
}


/*--------------------------------
 * Function: setcode
 * Parameters
 *         ix - index of alarm code to update
 * Description: Prompts user for a 4 digit alarm code to
 *              update the alarm code at index ix. If 'd'
 *              is entered the alarm code is disabled.  It
 *              is not allowed to diable the master
 *              alarm code.
 *-------------------------------*/
void setcode(byte ix)
{
   byte flag = TRUE;  // flag to control the loop
   byte i;  // loop index for input collection
   byte input;  // input from the user
   int digit;  // individual digit of the code
   int alarmCode = 0;  // stores the full alarm code
   int mult = 1000;  // multiplier to calculate the code value

   do
   {
       printLCDStr(GET_CODE_MSG,1);  // prompt user to enter the code
       for(i = 0; i < 4; i++)  // loop to get 4 digits of the code
       {
          input = readKey();  // read key input
          if(input == 'd')  // check for 'd' to exit or cancel code entry
          {
             if(ix == 0)  // if the index is 0 (master code), show error and break
             {
                printLCDStr(ERR_MST_MSG,1);  // display master code error message
                delayms(1000);  // wait for 1 second
                break;
             }
             else  // if it's not the master code, set alarmCode to invalid and exit
             {
                alarmCode = 0xffff;  // set invalid code value
                flag = FALSE;  // exit the loop
                break;
             }
          }
          else if(isdigit(input))  // if the input is a valid digit
          {
               digit = input - ASCII_CONV_NUM;  // convert input to integer
               alarmCode = alarmCode + digit * mult;  // build the full alarm code
               mult = mult / 10;  // reduce multiplier for next digit
               if(mult == 0) flag = FALSE;  // exit loop after all 4 digits are entered
          }
          else  // invalid input, show error and break
          {
              printLCDStr(CERRMSG,1);  // display input error message
              delayms(1000);  // wait for 1 second
              break;
          }
       }
   } while(flag);  // continue loop until valid code is entered or canceled

   if(writeToEE(ix, alarmCode)) /* do nothing */;  // store the code to EEPROM
}


/*------------------------------------
 * Function: initCodes
 * Description: If the alarm codes are not
 *              initialised (i.e. matercode is
 *              0xFFFF), initialise codes
 *              (that is, set master code
 *               to 0x0000).
 * --------------------------------*/
void initCodes()
{
    if(alarmCodes[0] == 0xFFFF)  // Assume erased
    {
      if(initEE(0,0x0000)) /*do nothing*/;  // initialises other word writes on success
    }
}

/*--------------------------------
 * Function: initEE
 * Parameters
 *         ix - index of alarm code to update
 *         code - code to store 
 * Description: Stores and integer alarm code to alarmcode
 *              array at index ix. Assume that the array
 *              is in EEPROM. Used for first write to the
 *              EEPROM.
 *-------------------------------*/
// Defines for use with controlling the EEPROM
#define ACCERR 0x10
#define PVIOL 0x20
#define CCIF 0x40
#define CBEIF 0x80
#define SECTOR_MODIFY 0x60
#define PROG 0x20

int initEE(int ix, int code)
{
   int retVal = TRUE;  // assume that write is successful
   int *eepromAddr = &alarmCodes[ix];
   
   ESTAT = ACCERR | PVIOL; // Clear error flags
   if(ESTAT & CBEIF)  // checks that command buffer is empty
   {
      *eepromAddr = code;    // Write data word aligned address
      ECMD = PROG;              // Write program command
      ESTAT = CBEIF;            // Write 1 to CBEIF to lauch command
      if(ESTAT & (ACCERR|PVIOL) != 0) retVal = FALSE; // Flag the error
      else while(!(ESTAT & CCIF)) ;  // wait until command complete      
   } 
   else retVal = FALSE; // Flag error - command buffer not empty
   return(retVal);
}

/*--------------------------------
 * Function: writeToEE
 * Parameters
 *         ix - index of alarm code to update
 *         code - code to store 
 * Description: Stores and integer alarm code to alarmcode
 *              array at index ix. Assume that the array
 *              is in EEPROM. Need to modify sector (4 bytes).
 *-------------------------------*/
int writeToEE(int ix, int code)
{
   int retVal = TRUE;  // assume that write is successful
   int *eepromAddr;
   int newcodes[2];
   // Detemine which four bytes need modifying
   if(ix < 2) // ix is 0 or 1
   {
      eepromAddr = &alarmCodes[0];
      newcodes[0] = alarmCodes[0];
      newcodes[1] = alarmCodes[1];
      newcodes[ix] = code;  // this is the modification
   } 
   else if(ix < 4)  // ix is 2 or 3 
   {
      eepromAddr = &alarmCodes[2];
      newcodes[0] = alarmCodes[2];
      newcodes[1] = alarmCodes[3];
      newcodes[ix-2] = code;  // this is the modification
   } 
   else
   {
      eepromAddr = &alarmCodes[4];
      newcodes[0] = code;
      newcodes[1] = 0xFFFF; // no 5th value in the array
   }
   
   ESTAT = ACCERR | PVIOL; // Clear error flags
   if(ESTAT & CBEIF)  // checks that command buffer is empty
   {
      // first word
      *eepromAddr = newcodes[0];    // Write data word aligned address
      ECMD = SECTOR_MODIFY;     // Erases 2 words, write one word
      ESTAT = CBEIF;            // Write 1 to CBEIF to lauch command
      if(ESTAT & (ACCERR|PVIOL) != 0) retVal = FALSE; // Flag the error
      else 
      {
         while(!(ESTAT & CCIF)) ;  // wait until command complete      
         *(eepromAddr+1) = newcodes[1];    // Write data word aligned address
         ECMD = PROG;              // Write program command
         ESTAT = CBEIF;            // Write 1 to CBEIF to lauch command
         if(ESTAT & (ACCERR|PVIOL) != 0) retVal = FALSE; // Flag the error
         else while(!(ESTAT & CCIF)) ;  // wait until command complete      
      }
   } 
   else retVal = FALSE; // Flag error - command buffer not empty
   return(retVal);
}



