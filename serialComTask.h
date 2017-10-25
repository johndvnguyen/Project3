#ifndef SERIALCOMTASK_H_
#define SERIALCOMTASK_H_

void communicate(void* data);
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount);
#endif