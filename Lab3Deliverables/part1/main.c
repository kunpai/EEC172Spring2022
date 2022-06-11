// Kunal Pai, Steven To

//*****************************************************************************
//
// Application Name     - IR Detection
// Application Overview - The objective of this application is to decode signals from an IR TV Remote
//
//*****************************************************************************

//****************************************************************************
//
//!
//!
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"

// Common interface includes
#include "uart_if.h"
#include "timer_if.h"
#include "gpio_if.h"

#include "pin_mux_config.h"

// macros of string sequences of button presses
#define ZERO "00000100101110110001100011100111"
#define ONE "00000100101110111000000001111111"
#define TWO "00000100101110110100000010111111"
#define THREE "00000100101110111100000000111111"
#define FOUR "00000100101110111010000001011111"
#define FIVE "00000100101110110110000010011111"
#define SIX "00000100101110111110000000011111"
#define SEVEN "00000100101110111001000001101111"
#define EIGHT "00000100101110110101000010101111"
#define NINE "00000100101110111101000000101111"
#define MUTE "00000100101110110010001011011101"
#define LAST "00000100101110111001001001101101"
#define PAUSE "00101100001110100" //0.2 ms 0 as 1 ms
#define PLAY "00101100001110110" // 0.2 ms as 0 everything else 1
#define STOP "00101100011110101"
#define FF "00101100001110110"
#define REW "00101100011110111"
#define VOLPLUS "00000100101110111000100001110111"
#define VOLMINUS "00000100101110110100100010110111"
#define CHANPLUS "00000100101110110000100011110111"
#define CHANMINUS "00000100101110111100100000110111"


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);
// Globals used to save and restore interrupt settings.

unsigned long interval = 0;
char reset = 0;
char input[33] = "";
int bitlen = 0;

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static PinSetting IR_IN = { .port = GPIOA2_BASE, .pin = 0x2}; // pin 08 is GPIO input

//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void BoardInit(void);

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************

char* decodeInput () {
    if(strcmp(ZERO, input) == 0){
        return "0";
    }
    else if (strcmp(ONE, input) == 0){
        return "1";
    }
    else if (strcmp(TWO, input) == 0){
        return "2";
    }
    else if (strcmp(THREE, input) == 0){
        return "3";
    }
    else if (strcmp(FOUR, input) == 0){
        return "4";
    }
    else if (strcmp(FIVE, input) == 0) {
        return "5";
    }
    else if (strcmp(SIX, input) == 0){
        return "6";
    }
    else if (strcmp(SEVEN, input) == 0){
        return "7";
    }
    else if (strcmp(EIGHT, input) == 0){
        return "8";
    }
    else if (strcmp(NINE, input) == 0) {
        return "9";
    }
    else if (strcmp(MUTE, input) == 0) {
        return "MUTE";
    }
    else if (strcmp(LAST, input) == 0) {
        return "LAST";
    }
    else if (strcmp(VOLPLUS, input) == 0) {
        return "VOLPLUS";
    }
    else if (strcmp(VOLMINUS, input) == 0) {
        return "VOLMINUS";
    }
    else if (strcmp(CHANPLUS, input) == 0) {
        return "CHANPLUS";
    }
    else if (strcmp(CHANMINUS, input) == 0) {
        return "CHANMINUS";
    }
    else
        return NULL;
}


void display(void)
{
    printf("%s\n", input);
    char* input = decodeInput();
    printf("%s\n", input);

    if (input != NULL) { // invalid button pressed
        Report("\r%s\n\r", input);
    }
}

void getData(void)
{
    if (interval >= 3) // 1.64 ms ~ > 3 times 0.5 ms
        strcat(input, "1");
    else
        strcat(input, "0");

    bitlen++;

    if (bitlen == 32) // end of signal
        display();

}

void TimerA0IntHandler(void) { // timer interrupt

    Timer_IF_InterruptClear(TIMERA0_BASE);
    interval++; // marking every 0.5 ms interval 
}



//*****************************************************************************
//
//! The interrupt handler for each rising and falling edge
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
static void GPIOA2IntHandler(void) {
    unsigned long ulStatus;
    long read;

    ulStatus = MAP_GPIOIntStatus (IR_IN.port, true);
    MAP_GPIOIntClear(IR_IN.port, ulStatus);     // clear interrupts on pin 8

    read = GPIOPinRead(IR_IN.port, IR_IN.pin);


    if (read == 0x2) {  //rising edge
        interval = 0;
        MAP_TimerEnable(TIMERA0_BASE, TIMER_A); // enable 0.5 ms interval counter

    } else if (read == 0) {  //falling edge

        if (reset == 1) {
                getData();
        }

        if (interval >= 8) { // reset signal received (long singal in beginning)
                input[0] = '\0';
                reset = 1;
                bitlen = 0;
        }
    }



}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void) {
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//
//! Main function
//!
//! \param none
//!
//!
//! \return None.
//
//****************************************************************************
int main() {
    unsigned long ulStatus;

    BoardInit();

    PinMuxConfig();

    InitTerm();

    ClearTerm();

    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC_UP, TIMER_A, 0);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerA0IntHandler);
    Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 40000);  //0.5ms
    // Register the interrupt handlers
    //
    MAP_GPIOIntRegister(IR_IN.port, GPIOA2IntHandler);

    //
    // Configure rising edge interrupt on IR_IN
    //
    MAP_GPIOIntTypeSet(IR_IN.port, IR_IN.pin, GPIO_BOTH_EDGES);

    ulStatus = MAP_GPIOIntStatus (IR_IN.port, false);
    MAP_GPIOIntClear(IR_IN.port, ulStatus);         // clear interrupts on GPIOA2

    // Enable GPIO Interrupt
    MAP_GPIOIntEnable(IR_IN.port, IR_IN.pin);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************