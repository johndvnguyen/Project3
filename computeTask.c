#include "dataPtrs.c"
#include "dataStructs.c"
#include "systemTimeBase.h"
#include "Flags.h"

void compute(void *data)
{

  // Cast struct
  computeData2* cData =(computeData2*)data;
  
  //find the current index of the array based on call count. 
  unsigned int index = ((*(cData->countCallsPtr)) % 8);
  
  // Transform raw temperature data to corrected temperature data
  (cData->tempCorrectedBufPtr[index]) = (unsigned char)( 5 + (0.75 * (cData->temperatureRawBufPtr[index])));
  //printf("tempCorrected: %i \n", cData->tempCorrectedBufPtr[index]);
  
  // Transform raw sys blood pressure data to corrected sys blood pressure data
  (cData->bloodPressCorrectedBufPtr[index]) = (unsigned char)(9 + (2 * (cData->bloodPressRawBufPtr[index])));
  //printf("sysCorrected: %i \n", cData->bloodPressCorrectedBufPtr[index]);
  
  // Transform raw dia blood pressure data to corrected dia blood pressure data
  (cData->bloodPressCorrectedBufPtr[index + 8]) = (unsigned char)(6 + (1.5 * (cData->bloodPressRawBufPtr[index + 8])));
  //printf("diaCorrected: %i \n", cData->bloodPressCorrectedBufPtr[index + 8]);
  
  // Transform raw pulse rate data to corrected pulse rate data
  //(cData->pulseRateCorrectedBufPtr[index + 8]) = (unsigned char)(8 + (3 * (cData->pulseRateRawBufPtr[index + 8])));
  //printf("PulseCorrected: %i \n", cData->pulseRateCorrectedBufPtr[index + 8]);
  
  // Designate for deletion
  computeFlag = 2;
  return;
}