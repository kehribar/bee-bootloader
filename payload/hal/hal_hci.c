/*-----------------------------------------------------------------------------
/ 
/	Button / LED related functions.
/
/------------------------------------------------------------------------------
/   ihsan Kehribar - 2013 
/----------------------------------------------------------------------------*/
#include "hal_hci.h"
/*---------------------------------------------------------------------------*/
void init_hci()
{
    /* init button 1 */
    pinMode(D,2,INPUT);
    internalPullup(D,2,ENABLE);

    /* init button 2 */
    pinMode(D,3,INPUT);
    internalPullup(D,3,ENABLE);

    /* init LEDs */
    pinMode(D,5,OUTPUT);
    digitalWrite(D,5,LOW); 
    pinMode(D,6,OUTPUT);
    digitalWrite(D,6,LOW); 
}
/*---------------------------------------------------------------------------*/
