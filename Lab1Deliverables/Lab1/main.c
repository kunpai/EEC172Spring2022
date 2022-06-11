// Kunal Pai, Steven To

//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - GPIO Application
// Application Overview - -	Use UArt to display the banner of the GPIO application along with the input pins and what they do. 
//                        -	Use GPIO functions to read inputs from two switches: SW2 and SW3:
//                              o	If SW3 is pressed, the three LEDs blink in a sequence from 000 to 111 with red being the least significant bit and green being the most significant bit. “SW3 pressed” is printed to the terminal.  
//                              o	If SW2 is pressed, all the three LEDs blink on and off in unison. “SW2 pressed” is printed to the terminal.  
//                        -	The output signal, P18, is set high whenever SW2 is pressed and low whenever SW3 is pressed.

//
//*****************************************************************************

//*****************************************************************************
//
//!
//
//*****************************************************************************

// Driverlib includes
#include <stdio.h>
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "uart.h"
#include "interrupt.h"
#include "pin_mux_config.h"
#include "utils.h"
#include "prcm.h"
#include "gpio.h"

// Common interface include
#include "gpio_if.h"
#include "uart_if.h"

//*****************************************************************************
//                          MACROS
//*****************************************************************************
#define APPLICATION_VERSION  "1.4.0"
#define APP_NAME             "GPIO"
#define CONSOLE              UARTA0_BASE


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

static void BoardInit(void);

//*****************************************************************************
//                      LOCAL DEFINITION
//*****************************************************************************
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void) //initialize the board
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************

static void DisplayBanner(char * AppName) // header that introduces the project
{

    Message("\n\n\n\r");
    Message("\t\t *************************************************\n\r");
    Message("\t\t        CC3200 %s Application       \n\r", AppName);
    Message("\t\t *************************************************\n\r");
    Message("\n\n\n\r");
    Message("\t\t Push SW3 to start LED binary counting \n\r");
    Message("\t\t Push SW2 to blink LEDs on and off \n\r") ;
}



//*****************************************************************************
//
//! Main function handling the uart echo. It takes the input string from the
//! terminal while displaying each character of string. whenever enter command
//! is received it will echo the string(display). if the input the maximum input
//! can be of 80 characters, after that the characters will be treated as a part
//! of next string.
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
int main()
{
    //
    // Initailizing the board
    //
    BoardInit();
    //
    // Muxing for Enabling UART_TX and UART_RX.
    //
    PinMuxConfig();
    // Initializing the Terminal
    InitTerm();
    // Clearing the Terminal
    ClearTerm();
    DisplayBanner("GPIO");
    GPIO_IF_LedConfigure(LED1|LED2|LED3);
    GPIO_IF_LedOff(MCU_ALL_LED_IND);
    int flag = 0; // 0 if no button, 1 after sw3 pressed for the first time and 2 after sw2 pressed for the first time
    int p18; // 0 if low, 1 if high
    while(1)
    {
        long sw3 = GPIOPinRead(GPIOA1_BASE, 0x20) >> 5; // SW3 is in bit 4 
        long sw2 = GPIOPinRead(GPIOA2_BASE, 0x40) >> 6; // SW2 is in bit 5

        if (((sw3 & 0x1) == 1) && flag != 1) {
            flag = 1;
            p18 = 0<<4;
            GPIOPinWrite(GPIOA3_BASE, 0x10, p18); // p18 as low
            Message("SW3 pressed\n\r");
            while (1)
            {

                GPIO_IF_LedOff(MCU_ALL_LED_IND);
                //
                // Turn LEDs on in a sequence of 000 to 111
                //
                for (int i = 0; i<8 ; i++) // LED for different numbers from 1 to 7
                {
                    MAP_UtilsDelay(8000000);
                    sw2 = GPIOPinRead(GPIOA2_BASE, 0x40) >> 6; // check if SW2 is pressed

                    if ((sw2 & 0x1) == 1)
                        break;
                    if (i == 1){
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                    }
                    if (i == 2){
                        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                    }
                    if (i == 3){
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                    }
                    if (i==4){
                        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
                        GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
                        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
                    }
                    if (i==5){
                        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                    }
                    if (i==6){
                        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                    }
                    if (i==7){
                        GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
                        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
                        GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
                    }
                    MAP_UtilsDelay(8000000);
                    GPIO_IF_LedOff(MCU_ALL_LED_IND);
                }
                if ((sw2 & 0x1) == 1)
                        break;

            }
        }
        
        
          if (((sw2 & 0x1) == 1) && flag != 2) {
            flag = 2;
            p18 = 1<<4;
            GPIOPinWrite(GPIOA3_BASE, 0x10, p18); // p18 as high
            Message("SW2 pressed\n\r");
            while (1)
            {
                GPIO_IF_LedOff(MCU_ALL_LED_IND);
                //
                // Turn all LEDs on and off
                //
                GPIO_IF_LedOn(MCU_ALL_LED_IND);
                MAP_UtilsDelay(8000000);
                GPIO_IF_LedOff(MCU_ALL_LED_IND);
                MAP_UtilsDelay(8000000);
                sw3 = GPIOPinRead(GPIOA1_BASE, 0x20) >> 5; // check if SW3 is pressed
                if ((sw3 & 0x1) == 1)
                    break;
            }
        }
    }
    return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************