#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "drivers/rit128x96x4.h"
#include "dataStructs.c"
#include "dataPtrs.h"

void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount)
{
    //
    // Loop while there are more characters to send.
    //
    while(ulCount--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPutNonBlocking(UART0_BASE, *pucBuffer++);
    }
}


/*
displays message on hyperterminal through uart.
Input:		A display values
Output: 	text on uart
Function:  	The following routine displays corrected information onto hyperterm.
*/

void communicate(void* data){
    communicationsData * cData = (communicationsData*)data;
    unsigned int index = ((*(cData->countCallsPtr)) % 8);
    unsigned int diaIndex = index + 8;
    
    //find the latest value of temperature
    unsigned char tempCorrect =(cData->tempCorrectedBufPtr[index]);
    
    char temp[20];
    sprintf(temp,"Temp %d C\n\r",tempCorrect);
    
    //print temp to screen
    UARTSend((unsigned char *)temp, 12);
    
    //find the latest value of systolic bp
    unsigned char sysCorrect =(cData->bloodPressCorrectedBufPtr[index]);
        
    //find the latest value of diastolic bp
    unsigned char diaCorrect =(cData->bloodPressCorrectedBufPtr[diaIndex]);
    
    char bP[20];
    sprintf(bP,"SYS %d mmHg\n\r",sysCorrect);
    
    //print systolic pressure to screen
    UARTSend((unsigned char *)bP,15);
    
    //print distolic pressure to screen
    sprintf(bP,"DIA %d mmHg\n\r",diaCorrect);
    UARTSend((unsigned char *)bP,15);
    
    //retrieve corrcted pulse rate
    unsigned char pulseCorrect =(cData->prCorrectedBufPtr[index]);
    
    char pulse[20];
    sprintf(pulse,"PR %d BPM\n\r",pulseCorrect);
    
    //retrieve correcte battery stat
    unsigned short battery = (*(cData->batteryStatePtr));
    
    char batt[20];
    sprintf(batt,"Batt %d\n\r",battery);
    
    //pront batt stat and pulse rate
    UARTSend((unsigned char *)pulse, 10);
    UARTSend((unsigned char *)batt, 12);
  


}
