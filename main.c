#include "inc/hw_types.h"
#include "computeTask.h"
#include "dataPtrs.h"
#include "displayTask.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "drivers/rit128x96x4.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "dataStructs.c"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/lm3s8962.h"
#include "measureTask.h"
#include "systemTimeBase.h"
#include "warningAlarm.h"
#include "serialComTask.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLOCK_RATE              300

//  Declare the globals
INIT_MEASUREMENT2(m2);
INIT_DISPLAY2(d2);
INIT_STATUS(s1);
INIT_ALARMS(a1);
INIT_WARNING(w1);
INIT_SCHEDULER(c1);
INIT_KEYPAD(k1);

//Connect pointer structs to data
measureData2 mPtrs2 = 
{     
  m2.temperatureRawBuf,
  m2.bloodPressRawBuf,
  m2.pulseRateRawBuf,
  &m2.countCalls,
  &m2.sysComplete,
  &m2.diaComplete,
  &m2.tempDirection,
  &m2.prDirection
};

computeData2 cPtrs2=
{
  m2.temperatureRawBuf,
  m2.bloodPressRawBuf,
  m2.pulseRateRawBuf,
  d2.tempCorrectedBuf,
  d2.bloodPressCorrectedBuf,
  d2.pulseRateCorrectedBuf,
  &k1.measurementSelection,
  &m2.countCalls
};

displayData2 dPtrs2=
{
  d2.tempCorrectedBuf,
  d2.bloodPressCorrectedBuf,
  d2.pulseRateCorrectedBuf,
  &s1.batteryState,
  &m2.countCalls,
  &k1.mode,
  &a1.tempOutOfRange,
  &a1.bpOutOfRange,
  &a1.pulseOutOfRange
  
};

warningAlarmData2 wPtrs2=
{
  m2.temperatureRawBuf,
  m2.bloodPressRawBuf,
  m2.pulseRateRawBuf,
  &s1.batteryState,
  &a1.bpOutOfRange,
  &a1.tempOutOfRange,
  &a1.pulseOutOfRange,
  &w1.bpHigh,
  &w1.tempHigh,
  &w1.pulseLow,
  &w1.led,
  &m2.countCalls,
  &w1.previousCount,
  &w1.pulseFlash,
  &w1.tempFlash,
  &w1.bpFlash,
  &w1.auralCount
};

statusData sPtrs=
{  
  &s1.batteryState
};

schedulerData schedPtrs=
{
  &c1.globalCounter
};

communicationsData comPtrs={
	d2.tempCorrectedBuf,
	d2.bloodPressCorrectedBuf,
	d2.pulseRateCorrectedBuf,
	&s1.batteryState,
	&m2.countCalls
};

//Declare the prototypes for the tasks
void compute(void* data);
void measure(void* data);
void stat(void* data);
void alarm(void* data);
void disp(void* data);
void schedule(void* data);
void buttonTest();

void insert(struct MyStruct* node);
struct MyStruct* head=NULL;
struct MyStruct* tail=NULL;
void delet(struct MyStruct* node);

unsigned volatile int globalCounter = 0;
//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the OLED display.
//
//*****************************************************************************
unsigned long g_ulFlags;



//*****************************************************************************
//
// A set of flags used to track the state of the application.
//
//*****************************************************************************

//extern unsigned long g_ulFlags;
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

//*****************************************************************************
//
// The speed of the processor.
//
//*****************************************************************************

unsigned long g_ulSystemClock;

//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     0 - Up
//     1 - Down
//     2 - Left
//     3 - Right
//     4 - Select
//
//*****************************************************************************

unsigned char g_ucSwitches = 0x1f;

//*****************************************************************************
//
// The vertical counter used to debounce the push buttons.  The bit positions
// are the same as g_ucSwitches.
//
//*****************************************************************************

static unsigned char g_ucSwitchClockA = 0;
static unsigned char g_ucSwitchClockB = 0;



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif
//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Update the global counter.
    //
    IntMasterDisable();
    increment();
    //annunciate(&wPtrs2);
    IntMasterEnable();
}

//*****************************************************************************
//
// Handles the SysTick timeout interrupt.
//
//*****************************************************************************

void
SysTickIntHandler(void)
{
    unsigned long ulData, ulDelta;

    // Indicate that a timer interrupt has occurred.
    HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 1;

    // Read the state of the push buttons.
    ulData = (GPIOPinRead(GPIO_PORTE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                            GPIO_PIN_2 | GPIO_PIN_3)) |
              (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) << 3));

    // Determine the switches that are at a different state than the debounced state.
    //debug line to imitate up click
    //ulData = 30;
    ulDelta = ulData ^ g_ucSwitches;

    // Increment the clocks by one.
    // Exclusive or of clock B If a bit is different in A and B then 1 if the bits have the same value = 0
    g_ucSwitchClockA ^= g_ucSwitchClockB;
    
    // Compliment of clock B. This changes 1 to 0 and 0 to 1 bitwise
    g_ucSwitchClockB = ~g_ucSwitchClockB;

    // Reset the clocks corresponding to switches that have not changed state.
    g_ucSwitchClockA &= ulDelta;
    g_ucSwitchClockB &= ulDelta;

    // Get the new debounced switch state.
    g_ucSwitches &= g_ucSwitchClockA | g_ucSwitchClockB;
    g_ucSwitches |= (~(g_ucSwitchClockA | g_ucSwitchClockB)) & ulData;

    // Determine the switches that just changed debounced state.
    ulDelta ^= (g_ucSwitchClockA | g_ucSwitchClockB);

    // See if any switches just changed debounced state.
    if(ulDelta)
    {
        // You can watch the variable for ulDelta
        // Up = 1 Right = 8 down =2 left =4  select = 16 Bit values
        // Add the current tick count to the entropy pool.
        printf("A button was pressed %d \n", ulDelta);
    }

    // See if the select button was just pressed.
    if((ulDelta & 0x10) && !(g_ucSwitches & 0x10))
    {
        // Set a flag to indicate that the select button was just pressed.
        HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
        //PWMGenDisable(PWM_BASE, PWM_GEN_0);
    }
}

 void main(void)
{  
  // Turn on LED to indicate normal state
  enableVisibleAnnunciation();
  
  // Initialize the OLED display.
  RIT128x96x4Init(1000000);
  
  //initialize scheduler
  schedule(&schedPtrs);
          
  return;

}


void schedule(void* data)
{
  // Counter to track the last time all tasks finished
  int previousCount = 0;
    
  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);

  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
 

  // Enable processor interrupts.
  IntMasterEnable();
  
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
     
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 460800,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
	
  // Configure the two 32-bit periodic timers.
  TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/10);

  // Setup the interrupts for the timer timeouts.
  IntEnable(INT_TIMER0A);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  // Enable the timers.
  TimerEnable(TIMER0_BASE, TIMER_A);
        
  //  Declare some TCBs 
  TCB displayT;
  TCB measureT;
  TCB statusT;
  TCB computeT;
  TCB warningT;
  TCB serialComT;
  
  
  //  Declare a working TCB pointer
  TCB* aTCBPtr;

  //Initialize the TCBs
  displayT.taskPtr = disp;
  displayT.taskDataPtr = (void*)&dPtrs2;
  
  measureT.taskPtr = measure;
  measureT.taskDataPtr = (void*)&mPtrs2;
  
  statusT.taskPtr = stat;
  statusT.taskDataPtr = (void*)&sPtrs;

  computeT.taskPtr = compute;
  computeT.taskDataPtr = (void*)&cPtrs2;

  warningT.taskPtr = alarm;
  warningT.taskDataPtr = (void*)&wPtrs2;
	
  
  serialComT.taskPtr= communicate;
  serialComT.taskDataPtr= (void*)&comPtrs;
  
  //Initialize the task queue
  insert(&measureT);
  insert(&warningT);
  insert(&statusT);
  insert(&computeT);
  insert(&displayT);
  insert(&serialComT);
  //insert(&scheduleT);

  //Dispatch the tasks
  while(1)
  {   
    // Reschedule tasks after 5 seconds have elapsed
    if(NULL==head && (globalCounter - previousCount >= 50)){
      //printf("\n\n\nSCHEDULING!\n\n\n");
      insert(&measureT);
      insert(&warningT);
      insert(&statusT);
      insert(&computeT);
      insert(&displayT);
      insert(&serialComT);
    }
        
    // Continue to annunciate while other tasks are delayed
    annunciate(&wPtrs2);
    
    // Execute task at head of linked list and then delete
    if(!(NULL==head))
    {
      aTCBPtr = head;
      aTCBPtr->taskPtr((aTCBPtr->taskDataPtr) );
      delet(head);
    
      if(NULL == head)
        previousCount = globalCounter;
    }
    buttonTest();
  }          
}


/*
Function insert
Input: A node from the linked list
Output: Null
Do: Inserts a node (task) into the linked list
*/
void insert(struct MyStruct* node)
{
  if(NULL==head){
    head=node;
    tail=node;
  }
  else
  {
    tail->next=node;
    node->prev =tail;
    tail=node;
  }
    return;
}
	
/*
Function delet
Input: A node from the linked list
Output: Null
Do: Deletes a node (task) into the linked list
*/
void delet(struct MyStruct* node)
{
  if(NULL==head)
  {
    return;
  }
  else if(head==tail)
  {
    head=NULL;
    tail=NULL;
  }
  else if(head==node)
  {
   head = head->next;
  }
  else if(tail==node)
  {
    tail=tail->prev;
  }
  else
  {
    node->prev->next=node->next;
    node->next->prev=node->prev;
    node->next=NULL;
    node->prev=NULL;
  }
  return;
}

void buttonTest(){
    
    // Get the system clock speed.
    g_ulSystemClock = SysCtlClockGet();

    // Enable the peripherals used by the application.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure the GPIOs used to read the state of the on-board push buttons.
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    // Configure SysTick to periodically interrupt.
    SysTickPeriodSet(g_ulSystemClock / CLOCK_RATE);
    SysTickIntEnable();
    SysTickEnable();

    // Throw away any button presses that may have occurred while the splash
    // screens were being displayed.
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;
};
