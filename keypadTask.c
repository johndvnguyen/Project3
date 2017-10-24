#include <stdio.h>
#include "keypadTask.h"
#include "dataPtrs.h"
#include "bool.h"
#include "inc/hw_types.h"

extern unsigned long g_ulFlags;//extern unsigned long g_ulFlags;
#define FLAG_CLOCK_TICK         0           // A timer interrupt has occurred
#define FLAG_CLOCK_COUNT_LOW    1           // The low bit of the clock count
#define FLAG_CLOCK_COUNT_HIGH   2           // The high bit of the clock count
#define FLAG_UPDATE             3           // The display should be updated
#define FLAG_BUTTON             4           // Debounced state of the button
#define FLAG_DEBOUNCE_LOW       5           // Low bit of the debounce clock
#define FLAG_DEBOUNCE_HIGH      6           // High bit of the debounce clock
#define FLAG_BUTTON_PRESS       7           // The button was just pressed
#define FLAG_ENET_RXPKT         8           // An Ethernet Packet received
#define FLAG_ENET_TXPKT         9           // An Ethernet Packet transmitted

extern unsigned char g_ucSwitches;

void keypadfunction(void* data)
{
    //cast the data to a keypadData struct
    keypadData * keypadDataPtr = (keypadData*) data;
    /*
    unsigned short mode; 0 - Menus or 1 - Acknowledge
    unsigned short measurementSelection; 0 - Blood Pressure 1 - Temperature 2 - Pulse Rate 
    unsigned short scroll; Only works in menu mode
    unsigned short selectChoice;
    unsigned short alarmAcknowledge;
    */
    //Check to see what button is pressed
    printf("Pressed button: %d \n", g_ucSwitches);
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;
      //Reset flag to 0
      
       
    
   

  return;
}