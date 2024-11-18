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

/*---------------------
 * Function: configCodes
 * Parameters: none
 * Returns: nothing
 * Description: Gets user to select alarm code to update/disable. Call
 *              setcode to update the alarm code.
 * ---------------------*/
void configCodes()
{
   byte ix;  // index for array
   byte input; // input from user.
   byte flag;

   if(enterMstCode())  // check if master code is valid
     do
     {
        printLCDStr(CONFIGMSG,1);  // display configuration message
        input = readKey();  // read user input
        flag = TRUE;
        if(input == 'a') setcode(0);  // set master code if 'a' is entered
        else if( input>='1' && input <= '4')  // if input is a number between 1 and 4
        {
          ix = input-ASCII_CONV_NUM;  // convert ASCII to index
          setcode(ix);  // set the corresponding alarm code
        }
        else 
        {
           printLCDStr(CERRMSG,1);  // display error message if input is invalid
           flag = FALSE;
        }
     } while(!flag);  // repeat until valid input is entered
}

/*--------------------------------
 * function: enterMstCode
 * parameters: none
 * returns: TRUE - if valid code entered, FALSE if not.
 * description: asks user for the 4 digit master alarm code.
 *-------------------------------*/
byte enterMstCode(void)
{
   byte retval = FALSE;  // default return value is FALSE
   byte i;
   byte input;
   int alarmCode = 0;
   int mult = 1000;  // multiplier for each digit (thousands, hundreds, etc.)
   
   printLCDStr(MSTCDMSG,1);  // display prompt for master code
   for(i=0 ; i<4 ; i++)  // loop through 4 digits of the code
   {
      input = readKey();  // read user input
      if(!isdigit(input)) break;  // if input is not a digit, break the loop
      else
      {
        alarmCode = alarmCode + mult*(input-ASCII_CONV_NUM);  // construct alarm code
        mult = mult/10;  // move to the next place value (hundreds, tens, ones)
        if(mult == 0)  // once all 4 digits are entered
        {
          if(alarmCode == alarmCodes[0]) retval = TRUE;  // check if the code matches the master code
        }
      }
   }
   return(retval);  // return whether the entered code was correct
}

/*--------------------------------
 * function: setcode
 * parameters
 *         ix - index of alarm code to update
 * description: prompts user for a 4 digit alarm code to
 *              update the alarm code at index ix. If 'd'
 *              is entered, the alarm code will be disabled.
 *              you can't disable the master alarm code.
 *-------------------------------*/
void setcode(byte ix)
{
   byte flag = TRUE;  // flag to control the loop
   byte i;
   byte input;
   int digit;
   int alarmCode=0;
   int mult=1000; // multiplier for each digit

   do
   {
       printLCDStr(GET_CODE_MSG,1);  // prompt user to enter the alarm code
       for(i=0 ; i< 4 ; i++)  // loop through 4 digits of the alarm code
       {
          input = readKey();  // read user input
          if(input == 'd')  // if 'd' is entered, disable the code
          {
             if(ix == 0) printLCDStr(ERR_MST_MSG,1);  // don't allow disabling master code
             else
             {
                alarmCode = 0xffff;  // set alarm code to disabled (0xffff)
                flag = FALSE;  // exit loop
                break;
             }
          }
          else if(isdigit(input))  // if input is a valid digit
          {
               digit = input - ASCII_CONV_NUM;  // convert input to integer
               alarmCode = alarmCode + digit * mult;  // construct alarm code
               mult = mult/10;  // move to the next place value
               if(mult == 0) flag = FALSE;  // once 4 digits are entered, exit loop
          }
          else
          {
              printLCDStr(CERRMSG,1);  // display error message for invalid input
              break;
          }
       }
   } while(flag);  // repeat until valid code is entered
   writeToEE(ix, alarmCode);  // store the code in EEPROM
}

/*------------------------------------
 * function: initCodes
 * description: if alarm codes haven't been set (i.e. master code
 *              is 0xFFFF), then initialize the codes (set master code 
 *              to 0x0000).
 * --------------------------------*/
void initCodes()
{
    if(alarmCodes[0] == 0xFFFF)  // check if the master code is uninitialized
       writeToEE(0,0x0000);  // initialize master code to 0x0000
}

/*--------------------------------
 * function: writeToEE
 * parameters
 *         ix - index of alarm code to update
 *         code - code to store 
 * description: saves the integer alarm code to the alarmCodes
 *              array at index ix, assumes array is in EEPROM.
 *-------------------------------*/
// defines for EEPROM control
#define ACCERR 0x10  // access error flag
#define PVIOL 0x20  // programming violation flag
#define CCIF 0x40  // command complete flag
#define CBEIF 0x80  // command buffer empty flag
#define PROG 0x20  // programming command

int writeToEE(int ix, int code)
{
   int retVal = TRUE;  // assume write is successful
   int *eepromAddr = &alarmCodes[ix];  // get address of EEPROM location
   
   ESTAT = ACCERR | PVIOL;  // clear any previous error flags
   if(ESTAT & CBEIF)  // check if command buffer is empty
   {
      *eepromAddr = code;  // write the code to the EEPROM address
      ECMD = PROG;  // set the program command
      ESTAT = CBEIF;  // clear the CBEIF to launch the command
      if(ESTAT & (ACCERR|PVIOL) != 0) retVal = FALSE;  // if error flags are set, mark failure
      else while(ESTAT & CCIF) ;  // wait for the command to complete      
   } 
   else retVal = FALSE;  // error - command buffer not empty
   return(retVal);  // return success or failure
}
