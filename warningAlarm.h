#include "bool.h"

#ifndef WARNINGALARM_H_
#define WARNINGALARM_H_
void alarm(void* data);
void checkWarnings(void* data);
void annunciate(void* data);
void checkPulse(unsigned int* pulse, Bool* pulseLow, int index, unsigned char* pulseOut);
void checkBp(unsigned int* bpBuf, Bool* bpHigh, int index, unsigned char* bpOut);
void checkTemp(unsigned int* temp, Bool* tempHigh, int index, unsigned char* tempOut);
void enableVisibleAnnunciation();
void disableVisibleAnnunciation();

#endif