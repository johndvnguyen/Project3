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
#include "serialComTask.h"
#include "systemTimeBase.h"
#include "warningAlarm.h"
#include "Flags.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

keypadData kPtrs=
{
  &k1.mode,
  &k1.measurementSelection,
  &k1.scroll,
  &k1.selectChoice,
  &k1.alarmAcknowledge

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
void keypadfunction(void* data);

void add(struct MyStruct* node);
void insert(struct MyStruct* current, struct MyStruct* new);
struct MyStruct* head=NULL;
struct MyStruct* tail=NULL;
struct MyStruct* nodeRef=NULL;
void delet(struct MyStruct* node);

unsigned volatile int globalCounter = 0;
unsigned int auralCounter = 0;
//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the OLED display.
//
//*****************************************************************************
unsigned long g_ulFlags;
unsigned long auralFlag;
unsigned long computeFlag;
unsigned long serialFlag;



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
    // only check buttons if there is not a button pressed
    if(!HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS)){
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
    
        
    
        // See if the select button was  pressed during an alarm.
        if(g_ucSwitches==15 && auralFlag==1)
        {
            
            // Set a flag to indicate that the select button was just pressed.
            //HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
            PWMGenDisable(PWM_BASE, PWM_GEN_0);
            auralFlag = 0;
            auralCounter = globalCounter;
        }
        // See if any switches just changed debounced state.
        if(ulDelta && (g_ucSwitches != 0x1F))
        {
            // You can watch the variable for ulDelta
            // Up = 1 Right = 8 down =2 left =4  select = 16 Bit values
    
            printf("A button was pressed %d \n", ulDelta);
            printf("SwitchesState %d \n", g_ucSwitches);
            HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
        }
    }
}

 void main(void)
{  
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
  auralFlag = 0;
  computeFlag = 1;
  serialFlag = 0;
  int queueEnd = 0;
  
  unsigned long ulPeriod;
    
  // Set the clocking to run directly from the crystal.
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_8MHZ);
  
  g_ulSystemClock = SysCtlClockGet();

  // Enable the peripherals used by this example.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  
  // Configure the GPIO used to output the state of the led
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

  //**INITIALIZE BUTTONS**//
  // Configure the GPIOs used to read the state of the on-board push buttons.
  GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                       GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  GPIOPadConfigSet(GPIO_PORTE_BASE,
                   GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD_WPU);
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

  // Configure SysTick to periodically interrupt.
  SysTickPeriodSet(g_ulSystemClock / CLOCK_RATE);
  SysTickIntEnable();
  SysTickEnable();
  
  //**INITIALIZE UART**//
  // Configure the GPIO for the UART
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
     
  // Set the configuration of the UART
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 460800,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
	
  //**INITIALIZE TIMER INTERRUPT**//
  // Configure the 32-bit periodic timer.
  TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
  TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/10);

  // Setup the interrupt for the timer timeout.
  IntEnable(INT_TIMER0A);
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  // Enable the timer.
  TimerEnable(TIMER0_BASE, TIMER_A);
  
  //**INITIAL SOUND WARNING**//
  // Set GPIO G1 as PWM pin.  They are used to output the PWM1 signal.
  GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
  
  // Compute the PWM period based on the system clock.
  ulPeriod = SysCtlClockGet() / 440;
  
  // Set the PWM period to 440 (A) Hz.
  PWMGenConfigure(PWM_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
  PWMGenPeriodSet(PWM_BASE, PWM_GEN_0, ulPeriod);

  // PWM1 to a duty cycle of 75%.
  PWMPulseWidthSet(PWM_BASE, PWM_OUT_1, ulPeriod * 3 / 4);

  // Enable the PWM1 output signal.
  PWMOutputState(PWM_BASE,PWM_OUT_1_BIT, true);
  
  // Enable processor interrupts.
  IntMasterEnable();
  
  // Turn on LED to indicate normal state
  enableVisibleAnnunciation();
        
  //  Declare some TCBs 
  TCB displayT;
  TCB measureT;
  TCB statusT;
  TCB computeT;
  TCB warningT;
  TCB serialComT;
  TCB keypadT;
  
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
  
  keypadT.taskPtr = keypadfunction;
  keypadT.taskDataPtr = (void*)&kPtrs;
	
  serialComT.taskPtr= communicate;
  serialComT.taskDataPtr= (void*)&comPtrs;
  
  //Initialize the task queue
  add(&measureT);
  add(&warningT);
  add(&statusT);
  add(&computeT);
  add(&displayT);
  add(&keypadT);
  
  // Assign the first task to head
  aTCBPtr = head;

  //Dispatch the tasks
  while(1)
  {   
    // Only annunciate for 5 seconds
    while(queueEnd && (globalCounter - previousCount <= 50))
    {
      annunciate(&wPtrs2);
    }
        
    // Continue to annunciate while other tasks are delayed
    annunciate(&wPtrs2);
    
    // Execute task at head of linked list
    if(!(NULL==head))
    {
      // Begin measurement to empirically measure task
      //clock_t begin = clock();
      aTCBPtr->taskPtr((aTCBPtr->taskDataPtr) );
      
      // End measurement to emprically measure task
      //clock_t end = clock();
      
      // Find the task time
      //double time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
      //printf("%f \n\n", time_spent);
      
      // Check if compute needs to be scheduled
      if(computeFlag == 1)
      {
        computeFlag = 0;
        insert(aTCBPtr, &computeT);
       
      }
      // Check if comput needs to be deleted
      if(computeFlag == 2)
      {
        computeFlag = 0;
        
        // Reference to next node
        nodeRef = aTCBPtr->next;
        delet(aTCBPtr);
      }
      
      // Check if serial comm needs to be scheduled
      if(serialFlag == 1)
      {
        serialFlag = 0;
        insert(aTCBPtr, &serialComT);
       
      }
      // Check if serial comm needs to be deleted
      if(serialFlag == 2)
      {
        serialFlag = 0;
        
        // Reference to next node
        nodeRef = aTCBPtr->next;
        delet(aTCBPtr);
      }
      
      // Wrap to begginning of queue
      if(aTCBPtr == tail)
      {
        aTCBPtr = head;
        queueEnd = 1;
        previousCount = globalCounter;
      }
      // Assign task pointer to node reference
      else if(aTCBPtr->next == NULL)
      {
        aTCBPtr = nodeRef;
      }
      // Move to next task in queue
      else
      {
      aTCBPtr = aTCBPtr->next;
      }
    }
    //buttonTest();
  }          
}

/*
Function add
Input: A node from the linked list
Output: Null
Do: Addss a node (task) into the linked list
*/
void add(struct MyStruct* node)
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
Function insert
Input: The current node from the linked list and the node to be inserted
Output: Null
Do: Inserts a node (task) into the linked list
*/
void insert(struct MyStruct* current, struct MyStruct* new)
{
  new->next = current->next;
  current->next = new;
  new->prev = current;
  
  if(new->next != NULL)
    new->next->prev = new;
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

    // Throw away any button presses that may have occurred while the splash
    // screens were being displayed.
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;
};
