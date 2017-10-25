/*
Citation:
Code to enable and disable light referenced from Blinky.c from StellarisWare example package.
*/

#include "inc/lm3s8962.h"
#include "warningAlarm.h"
#include "dataPtrs.c"
#include <stdlib.h>
#include <stdio.h>
#include "bool.h"
#include "systemTimeBase.h"
#include "driverlib/pwm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "Flags.h"

/*
Function alarm
Input: pointer to alarmData
Output Null
Do: Checks if vitals are out of range
*/
void alarm(void *data)
{
  checkWarnings(data);
  annunciate(data);
  auralAnnunciate(data);
}

/*
Function checkWarnings
Input pointer to alarmData
Output Null
Do: Checks raw measurements against ranges and annunciates accordingly
*/

void checkWarnings(void *data)
{
  warningAlarmData2 * alarm = (warningAlarmData2*) data;
  
  //find the current index of the array based on call count. 
  unsigned int index = ((*(alarm->countCallsPtr)) % 8);
  
  unsigned int* tempBuf = (*alarm).temperatureRawBufPtr;
  unsigned int* bpBuf = (*alarm).bloodPressRawBufPtr;
  unsigned int* pulseBuf = (*alarm).pulseRateRawBufPtr;
  unsigned char* bpOut = (*alarm).bpOutOfRangePtr;
  unsigned char* tempOut = (*alarm).tempOutOfRangePtr;
  unsigned char* pulseOut = (*alarm).pulseOutOfRangePtr;
  Bool* bpHigh = (*alarm).bpHighPtr;
  Bool* tempHigh = (*alarm).tempHighPtr;
  Bool* pulseLow = (*alarm).pulseLowPtr;

  // Check vitals against prescribed ranges. Set warnings accordingly
  checkTemp(tempBuf, tempHigh, index, tempOut);
  checkBp(bpBuf, bpHigh, index, bpOut);
  checkPulse(pulseBuf, pulseLow, index, pulseOut);
  return;
}



/*
Function checkTemp
Input: pointer to temperatureRaw, pointer to tempHigh
Output: Null
Do: Checks if values are within normal range and sets bool accordingly.
*/
void checkTemp(unsigned int* temp, Bool* tempHigh, int index, unsigned char* tempOut)
{
  // Check if temperature is in range. Set warning accordingly
  if((temp[index]) < 41.46 || (temp[index]) > 43.73)
  {
    *tempHigh = TRUE;
    *tempOut = 89;
  } 
  else
  {
    *tempHigh = FALSE;
    *tempOut = 78;
  } 
}

/*
Function checkBp
Input: pointer to systolicRaw, pointer to diastolicRaw, pointer to bpHigh
Output: Null
Do: Checks if values are within normal range and sets bool accordingly.
*/
void checkBp(unsigned int* bpBuf, Bool* bpHigh, int index, unsigned char* bpOut)
{
  // Check if blood pressure is in range.  Set warnings accordingly
  if ((bpBuf[index]) > 60.5 || (bpBuf[index]) < 55.5 || (bpBuf[index + 8]) > 49.33 || (bpBuf[index + 8]) < 42.67)
  {
    *bpHigh = TRUE; 
    *bpOut = 89;
  }
  else
  {
    *bpHigh = FALSE;
    *bpOut = 78;
  }
}

/*
Function checkPulse
Input: pointer to pulseRateRaw, pointer to pulseLow
Output: Null
Do: Checks if values are within normal range and sets bool accordingly.
*/
void checkPulse(unsigned int* pulse, Bool* pulseLow, int index, unsigned char* pulseOut)
{
  // Check if pulse rate is in range. Set warning accordingly.
  if ((int)(*pulse) < 60)
  {
    *pulseLow = TRUE;
    *pulseOut = 89;
  }
  else
  {
    *pulseLow = FALSE;
    *pulseOut = 78;
  }
}

void auralAnnunciate(void *data)
{
  warningAlarmData2 * alarm = (warningAlarmData2*) data;
  if((*(alarm->tempHighPtr)) || (*(alarm->pulseLowPtr)) || (*(alarm->bpHighPtr)))
  {
    // updated from 50 to 5000 to test without alarms going off
    if(auralFlag == 0 && (globalCounter - auralCounter >= 50))
    {
      auralFlag = 1;
      PWMGenEnable(PWM_BASE, PWM_GEN_0);
    }
  } 
  
  else
  {
    if(auralFlag == 1)
    {
      auralFlag = 0;
      PWMGenDisable(PWM_BASE, PWM_GEN_0);
    }
  }
  
}
/*
Function: annunciate
Input: warning data
Output: Flashing of LED on board
Do: Flashes LED at rate per specific warning.
*/
void annunciate(void *data)
{
  warningAlarmData2 * alarm = (warningAlarmData2*) data;
  unsigned int* led = (*alarm).ledPtr;
  unsigned long* previousCount = (*alarm).previousCountPtr;
  const long pulseFlash = *(alarm->pulseFlashPtr);
  const long tempFlash = *(alarm->tempFlashPtr);
  const long bpFlash = *(alarm->bpFlashPtr);
  
  // Flash at the correct rate for each warning.
      if(*(alarm->tempHighPtr)) //if(*(alarm->pulseLowPtr))
      { 
        if(globalCounter - (*previousCount) >= pulseFlash)
        {
          
          (*previousCount) = globalCounter;
          //printf("PREVIOUS COUNT: %i \n", (*previousCount));
          //printf("PULSELOW LED \n\n");
          if((*led) == 1)
          {
            //printf("LED OFF \n\n");
            disableVisibleAnnunciation();
            (*led) = 0;
          }
          else
          {
            //printf("LED ON \n\n");
            enableVisibleAnnunciation();
            (*led) = 1;
          }
        }    
      }
      else if (*(alarm->pulseLowPtr))
      {
        if(globalCounter - (*previousCount) >= tempFlash)
        { 
          (*previousCount) = globalCounter;
          //printf("PREVIOUS COUNT: %i \n", (*previousCount));
          //printf("TEMPHIGH LED \n\n");
          if((*led) == 1)
          {
            disableVisibleAnnunciation();
            (*led) = 0;
          }
          else
          {
            enableVisibleAnnunciation();  
            (*led) = 1;
          }
        }
      }
      else if (*(alarm->bpHighPtr))
      {
        if(globalCounter - (*previousCount) >= bpFlash)
        {
          (*previousCount) = globalCounter;
          printf("BPHIGH LED \n\n");
          if((*led) == 1)
          {
            disableVisibleAnnunciation();
            (*led) = 0;
          }
          else
          {
            enableVisibleAnnunciation();
            (*led) = 1;
          }
        }
      }
      return;
 
}

/*
Function enableVisibleAnnunciation
Input: N/A
Output: Null
Do: Turns on LED on StellarisWare board
*/
void enableVisibleAnnunciation()
{
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x01);
  return;
}

/*
Function disableVisibleAnnunciation
Input: N/A
Output: Null
Do: Turns off LED on StellarisWare board
*/

void disableVisibleAnnunciation()
{
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00);
  return;
}

