#include "displayTask.h"
#include "dataStructs.c"
#include "dataPtrs.c"
#include "drivers/rit128x96x4.h"
#include "systemTimeBase.h"
#include "utils/ustdlib.c"

void disp(void* data)
{  
    displayData2 * dData = (displayData2*)data;
    
    // If mode is 0, display main
    if(*(dData->modePtr) == 0)
    {
      RIT128x96x4Clear();
      mainDisp();
      *dData->modePtr = 1;
    }
    // If mode is 1, display menu
    else if(*(dData->modePtr) == 1)
    {
      RIT128x96x4Clear();
      
      menuDisp(data);
      *dData->modePtr = 2;
    }
    // If mode is 2, display annunciation menu
    else if(*(dData->modePtr) == 2)
    {
      RIT128x96x4Clear();
      
      annunciateDisp(data);
      *dData->modePtr = 0;
    }
    
    return;
}

void annunciateDisp(void* data)
{
  //printf("\n CHECKING DISPLAY! \n");
  displayData2 * dData = (displayData2*)data;
  
  //find the current index of the array based on call count. 
  unsigned int index = ((*(dData->countCallsPtr)) % 8);
  unsigned int diaIndex = index + 8;
  
  //find the latest value of temperature
  unsigned char tempCorrect =(dData->tempCorrectedBufPtr[index]);
  
  char temp[20];
  //sprintf(temp,"Temp %d C",tempCorrect);
  usprintf(temp,"Temp %d C",tempCorrect);

  
  //find the latest value of systolic bp
  unsigned char sysCorrect =(dData->bloodPressCorrectedBufPtr[index]);
      
  //find the latest value of diastolic bp
  unsigned char diaCorrect =(dData->bloodPressCorrectedBufPtr[diaIndex]);
  
  char bP[35];
  sprintf(bP,"Sys/Dia %d/%d mmHg",sysCorrect,diaCorrect);
  
  //find the latest value of pulse rate
  unsigned char pulseCorrect =(dData->pulseRateCorrectedBufPtr[index]);
  
  char pulse[20];
  sprintf(pulse,"PR %d BPM",pulseCorrect);
  
  //find the latest value of battery
  unsigned short battery = (*(dData->batteryStatePtr));
  
  char batt[20];
  sprintf(batt,"Batt %d ",battery);
  
  // Check for Warnings
  // Temp Warning
  char tempOut[20];
  if(*(dData->tempOutOfRangePtr) == 'Y')
  {
    usprintf(tempOut,"TEMP OUT OF RANGE!");
  }
  else if (*(dData->tempOutOfRangePtr) == 'N')
  {
    usprintf(tempOut,"");
  }
  
  // Blood Pressure Warning
  char bpOut[20];
  if(*(dData->bpOutOfRangePtr) == 'Y')
  {
    usprintf(bpOut,"BP OUT OF RANGE!");
  }
  else if (*(dData->bpOutOfRangePtr) == 'N')
  {
    usprintf(bpOut,"");
  }
  // Blood Pressure Warning
  char pulseOut[20];
  if(*(dData->pulseOutOfRangePtr) == 'Y')
  {
    usprintf(pulseOut,"PULSE OUT OF RANGE!");
  }
  else if (*(dData->pulseOutOfRangePtr) == 'N')
  {
    usprintf(pulseOut,"");
  }

  
  // Update OLED screen to show new values
  RIT128x96x4StringDraw(temp,5,9,15);
  RIT128x96x4StringDraw(bP,5,20,15);
  RIT128x96x4StringDraw(pulse,5,30,15);
  RIT128x96x4StringDraw(batt,80,9,15);
  RIT128x96x4StringDraw(tempOut,5,40,15);
  RIT128x96x4StringDraw(bpOut,5,50,15);
  RIT128x96x4StringDraw(pulseOut,5,60,15);
  

  return; 
}

void mainDisp()
{
  char menu[20];
  //sprintf(temp,"Temp %d C",tempCorrect);
  usprintf(menu,"Menu");
  
  char annun[20];
  //sprintf(temp,"Temp %d C",tempCorrect);
  usprintf(annun,"Annunciate");
 
  // Update OLED screen to show new values
  RIT128x96x4StringDraw(menu,5,9,15);
  RIT128x96x4StringDraw(annun,5,20,15);
}

void menuDisp()
{
  char bp[20];
  usprintf(bp,"Blood Pressure");
  
  char temp[20];
  usprintf(temp,"Temperature");
  
  char pulse[20];
  usprintf(pulse,"Pulse Rate");
 
  // Update OLED screen to show new values
  RIT128x96x4StringDraw(bp,5,9,15);
  RIT128x96x4StringDraw(temp,5,20,15);
  RIT128x96x4StringDraw(pulse,5,30,15);
}