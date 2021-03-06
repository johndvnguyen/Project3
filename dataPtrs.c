#include "bool.h"

// Holds pointers to the variables associated with measure task:
typedef struct
{
  unsigned int* temperatureRawBufPtr;
  unsigned int* bloodPressRawBufPtr;
  unsigned int* pulseRateRawBufPtr;
  unsigned int* countCallsPtr;
  //Variables to simulate data measurements
  unsigned int* sysCompletePtr;
  unsigned int* diaCompletePtr;
  int* tempDirectionPtr;
  unsigned long* prPtr;
} measureData2;

// Holds pointers to the variables associated with compute task:
typedef struct 
{
  unsigned int* temperatureRawBufPtr;
  unsigned int* bloodPressRawBufPtr;
  unsigned int* pulseRateRawBufPtr;
  unsigned char* tempCorrectedBufPtr;
  unsigned char* bloodPressCorrectedBufPtr;
  unsigned char* pulseRateCorrectedBufPtr;
  unsigned short* measurementSelectionPtr;
  unsigned int* countCallsPtr;
}computeData2;

// Holds pointers to the variables associated with display task:
typedef struct 
{
  unsigned char* tempCorrectedBufPtr;
  unsigned char* bloodPressCorrectedBufPtr;
  unsigned char* pulseRateCorrectedBufPtr;
  unsigned short* batteryStatePtr;
  unsigned int* countCallsPtr;
  unsigned short* modePtr;
  unsigned char* tempOutOfRangePtr;
  unsigned char* bpOutOfRangePtr;
  unsigned char* pulseOutOfRangePtr;
}displayData2;

// Holds pointers to the variables associated with warning/alarm task:
typedef struct
{
  unsigned int* temperatureRawBufPtr;
  unsigned int* bloodPressRawBufPtr;
  unsigned int* pulseRateRawBufPtr;
  unsigned short* batteryStatePtr;
  unsigned char* bpOutOfRangePtr;
  unsigned char* tempOutOfRangePtr;
  unsigned char* pulseOutOfRangePtr;
  Bool* bpHighPtr;
  Bool* tempHighPtr;
  Bool* pulseLowPtr;
  unsigned int* ledPtr;
  unsigned int* countCallsPtr;
  unsigned long* previousCountPtr;
  const long* pulseFlashPtr;
  const long* tempFlashPtr;
  const long* bpFlashPtr;
  unsigned long* auralCountPtr;
} warningAlarmData2;

// Holds pointers to the variables associated with status task:
typedef struct 
{
  unsigned short* batteryStatePtr;
}statusData;

// Holds pointers to the variables associated with scheduler task:
typedef struct
{
  unsigned int* globalCounterPtr;
}schedulerData;

// Holds pointers to the variables associated with keypad task:
typedef struct
{
  unsigned short* modePtr;
  unsigned short* measurementSelectionPtr;
  unsigned short* scrollPtr;
  unsigned short* selectChoicePtr;
  unsigned short* alarmAcknowledge;
}keypadData;

// Holds pointers to the variables associated with communications task:
typedef struct
{
  unsigned char* tempCorrectedBufPtr;
  unsigned char* bloodPressCorrectedBufPtr;
  unsigned char* prCorrectedBufPtr;
  unsigned short* batteryStatePtr;
  unsigned int* countCallsPtr;
}communicationsData;
