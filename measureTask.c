// Main and measureTask
#include <stdio.h>
#include "measureTask.h"
#include "dataPtrs.h"
#include "bool.h"
#include "systemTimeBase.h"
#include "Flags.h"


void measure(void* data)
{
    measureData2 * measureDataPtr = (measureData2*) data;
    
    measureTempArray(data);
    measureSysBPArray(data);
    measureDiaBPArray(data);
    measurePRArray(data);

    //Moved this to after the measurements so we start at index 0
    //increment the count entry
    ++(*(*measureDataPtr).countCallsPtr);
    
    // If compute task is already in queue don't add
    if(computeFlag == 1)
    {
      computeFlag = 0;
    }
    // Add compute to queue if new measurement is made
    else
    {
    computeFlag = 1;
    }

  return;
}

/*
Function measureTempArray
Input pointer to measureData
Output Null
Do: updates the tempRaw based on algorithm
*/
void measureTempArray(void* data){
  measureData2* measureDataPtr = (measureData2*) data;
  //printf("This is a measureTemp Function \n");
  //Check to see if the temperature is increasing or decreasing
  int* direction = (*measureDataPtr).tempDirectionPtr;
  
  //Creates a local pointer to the countCalls Variable
  unsigned int* countCalls = (*measureDataPtr).countCallsPtr;
  //Creates a local pointer to the start of the array
  unsigned int* tempRawBuf = (*measureDataPtr).temperatureRawBufPtr;
  
  //find the current index of the array based on call count. 
  unsigned int index = (*countCalls) %8;
  unsigned int next = (*countCalls +1) %8;
  //If temperature is above 50 and increasing swap the direction
  if (50<=tempRawBuf[index] && 1 == *direction){
    *direction = -1;
  }
  //If temperature is below 15 and decreasing swap the direction
  else if (15>=tempRawBuf[index] && -1 == *direction){  
    *direction = 1;
  }
  // increment or decrement (using the direction value) If even the magnitude is 2 if odd the magnitude is 1
  //printf("TempRawBefore = %d \n", tempRawBuf[index]);
  tempRawBuf[next] = tempRawBuf[index] + (*direction) * (((*countCalls + 1) % 2) + 1);
  //printf("TempRawAfter = %d \n",tempRawBuf[next]);
};

/*
Function measureSysBp
Input pointer to measureData
Output Null
Do: Places Systolic into array indexes 0-7
*/
void measureSysBPArray(void* data){
    measureData2* measureDataPtr = (measureData2*) data;
    //printf("This is a measureSysBp Function \n");
    //Check to see if the DiaBp is complete and repeat the original proces
    unsigned int* countCalls = (*measureDataPtr).countCallsPtr;
    unsigned int* bloodPressRawBuf  = (*measureDataPtr).bloodPressRawBufPtr;
    unsigned int* sysComplete = (*measureDataPtr).sysCompletePtr;
    unsigned int* diaComplete = (*measureDataPtr).diaCompletePtr;
    //find the current index of the array based on call count. 
    unsigned int sysLast = (*countCalls) %8;
    unsigned int sysNext = (*countCalls +1) %8;
    
    if (1==*diaComplete && bloodPressRawBuf[sysLast]>100){
      bloodPressRawBuf[sysNext] = 80;
      *diaComplete = 0;
    }
    // If the sysBP <= 100 its not complete so we increment it
    if (100 >= bloodPressRawBuf[sysLast] ){
      if  ( (*countCalls % 2) == 0){
        bloodPressRawBuf[sysNext] = bloodPressRawBuf[sysLast] + 3;
      }
      else{
        bloodPressRawBuf[sysNext] = bloodPressRawBuf[sysLast] - 1;
      }
    }
    // If sysBP > 100 it is complete and we wait til diaCompletes
    if (100 < bloodPressRawBuf[sysNext]){
      *sysComplete = 1;
    }
};

/*
Function measureDiaBp
Input pointer to measureData
Output Null
Do: Places Systolic into array indexes 8-15
*/
void measureDiaBPArray(void* data){
  
    measureData2* measureDataPtr = (measureData2*) data;
    unsigned int* countCalls = (*measureDataPtr).countCallsPtr;
    unsigned int* bloodPressRawBuf = (*measureDataPtr).bloodPressRawBufPtr;
    unsigned int* sysComplete = (*measureDataPtr).sysCompletePtr;
    unsigned int* diaComplete = (*measureDataPtr).diaCompletePtr;
    unsigned int diaLast = ((*countCalls) %8) + 8;
    unsigned int diaNext = ((*countCalls +1) %8) + 8;
   // printf("This is a measureSysBp Function \n");
  //Check to see if the DiaBp is complete and repeat the original proces
     if (1==*sysComplete && bloodPressRawBuf[diaLast]<40){
      bloodPressRawBuf[diaNext] = 80;
      *sysComplete = 0;
      }
    // If diastolyic BP is above 40 it is not complete
    if (40 <= bloodPressRawBuf[diaLast]){
      if  ( ((*countCalls) % 2) == 0){
        bloodPressRawBuf[diaNext] = bloodPressRawBuf[diaLast] - 2;
      }
      else{
        bloodPressRawBuf[diaNext] = bloodPressRawBuf[diaLast] + 1;
      }
    } 
    // diastolyic BP drops below 40 and is complete
    if (40 > bloodPressRawBuf[diaNext]){
      *diaComplete = 1;
    }
};


/*
Function measurePrArray
Input pointer to measureData
Output Null
Do: Needs to be updated with the model transducer handling.
*/
void measurePRArray(void* data){
 int check=0;
    int beatCount=0;
    int bpm=0;
    int change =0;
    measureData2* measureDataPtr = (measureData2*) data;
    unsigned int clock = globalCounter;//(*measureDataPtr).globalCounterPtr;
    unsigned int temp=clock;
    while(clock<(temp+1)){
        
        clock=globalCounter;//(*measureDataPtr).globalCounterPtr;
        unsigned long* beat=(*measureDataPtr).prPtr;
        
        if(*beat==1 && check==0){
            beatCount++;
            check=1;
        }
        else if(*beat==0){
           check=0;
        }
    }
    
    bpm=(beatCount*60/3);
    
    unsigned int* countCalls = (*measureDataPtr).countCallsPtr;
    unsigned int* pulseRateRawBuf = (*measureDataPtr).pulseRateRawBufPtr;
    unsigned int prLast = (*countCalls) %8;
    unsigned int prNext = (*countCalls+1) %8;
    //Check to see if we prLast would cause divide by 0
    if(pulseRateRawBuf[prLast]!=0){
      change = ((pulseRateRawBuf[prLast]-bpm)*100)/(pulseRateRawBuf[prLast]);
    }
    //If the last value was 0, we shouuld have change be 0
    //Each measurement does not currently have its own counter 
    //so the display task only looks at an index across three
    else{
      change = 15;
    }
    if(change>=15||change<=-15){
        pulseRateRawBuf[prNext]=bpm;
    }

}
