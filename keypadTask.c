#include <stdio.h>
#include "keypadTask.h"
#include "dataPtrs.h"
#include "bool.h"
#include "inc/hw_types.h"
#include "drivers/rit128x96x4.h"
#include "Flags.h"

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
    unsigned short mode; 0 none - 1 Menus or 2 - Annunciate
    unsigned short measurementSelection; 0 - Blood Pressure 1 - Temperature 2 - Pulse Rate 
    unsigned short scroll; Only works in menu mode
    unsigned short selectChoice;
    unsigned short alarmAcknowledge;
    */
    unsigned short * modePtr = (*keypadDataPtr).modePtr;
    unsigned short * measurementSelectionPtr = (*keypadDataPtr).measurementSelectionPtr;
    unsigned short * selectChoicePtr = (*keypadDataPtr).selectChoicePtr;
    unsigned int row;
    int choice = 0;
    //Mode selection (Mode 0, main menu)
    if(*modePtr == 0){
      
      //down is pressed and the cursor is not on annunciate
      if(g_ucSwitches==29 && *selectChoicePtr!=2){
        *selectChoicePtr = 2;
      }
      //else up is pressed and cursor is not on menu
      else if(g_ucSwitches==30 && *selectChoicePtr!=1){
        *selectChoicePtr = 1;
      }
      //Check to see if a selection was made
      if(g_ucSwitches==15){
        //update the mode selection
        *modePtr = *selectChoicePtr;
        //update the choice back to one
        *selectChoicePtr=1;
        choice = 1;
      }
      
      // if modechoice is hovering on annunciation show cursor in second row
      // if not show it in the first row (This is to help with initializing subsquent menus)
      if (*selectChoicePtr==2){
        row = 20;
      }else{
        row = 9;
      }
    }
    //Measurement selection (Mode 1 for menu)
    if(*modePtr == 1 && choice != 1){

      //down is pressed and the cursor is not on annunciate
      if(g_ucSwitches==29 && *selectChoicePtr!=3){
        (*selectChoicePtr) += 1;
      }
      //else up is pressed and cursor is not on menu
      else if(g_ucSwitches==30 && *selectChoicePtr!=1){
        (*selectChoicePtr) -= 1;
      }
      //Check to see if a selection was made
      if(g_ucSwitches==15&& auralFlag==0){
        //update the measurement selection
        *measurementSelectionPtr = *selectChoicePtr;
        *modePtr = 0;
        choice = 1;
      }
       
      // if modechoice is hovering on annunciation show cursor in second row
      // if not show it in the first row (This is to help with initializing subsquent menus)
      switch(*selectChoicePtr){
        case(1):
          row = 10;
          break;
        case(2):
          row = 20;
          break;
        case(3):
          row = 30;
          break;
      }
    }
    
    // Char to hold the cursor symbol
    char cursor[20];
    usprintf(cursor,"*");
    
    //display updated cursor
    // The annunciate mode does not need a cursor
    if(*modePtr !=2){
      
      RIT128x96x4StringDraw(cursor,5,row,15);
    }
    
    // If in annunciate and select is pressed set mode to main menu
    if(*modePtr == 2 && choice != 1){
      if(g_ucSwitches==15&& auralFlag==0){
        *modePtr = 0;
      }
    }
   
    //Reset button pressed flag to 0 if set
    if(HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS)){
      HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;  
    }
  return;
}