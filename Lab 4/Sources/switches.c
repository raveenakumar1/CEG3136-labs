/*-------------------------------------------------------------
 * File:  switches.c
 * Description: This file contains the Switches module for the
 *              Alarm System Simulation project.
-----------------------------------------------------------------*/
#include "switches.h"  // Definitions file

/*----------------------------------------
 * Function: initSwitches
 * Parameters: none
 * Returns: nothing
 * Description: Initialises the port for monitoring the switches.
 *----------------------------------------*/
void initSwitches()
{         
   DDRH = 0;      // configure Port H as input (for switches)
   PERH = 0xff;   // enable pull-up/pull-down resistors on Port H pins
   PPSH = 0;      // use pull-down resistors for switches on Port H
                 // switches connect to ground when closed, enabling detection of low state
}
/*------------------------
 * Function: getSwStatus
 * Parameters:  none
 * Returns: An 8 bit code that indicates which
 *          switches are opened (bit set to 1).
 * Description: Checks status of switches and 
 *              returns bytes that shows their
 *              status.      
 *---------------------------*/
byte getSwStatus()
{
    return(PTH);
}
