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
// Application Name     - I2C 
// Application Overview - To develop an application program that involves the use of 
//                        SPI to connect the OLED with the CC3200 Launchpad and I2C 
//                        to obtain tilt readings from the BMA222
//                        For this program, a ball is first displayed at the center 
//                        of the OLED and can travel in the x or y directions (or both) 
//                        based on the tilt readings of the BMA222. The greater the tilt 
//                        in any direction, the faster the ball will travel in said direction, 
//                        stopping when it hits the edges of the OLED display.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup i2c_demo
//! @{
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"

// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"
#include "spi.h"
#include "test.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
 
#include "pin_mux_config.h"


//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "I2C Demo"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}
#define SPI_IF_BIT_RATE         100000

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


//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                          
//****************************************************************************

//*****************************************************************************
//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t      CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}


// //*****************************************************************************
// //
// //! Board Initialization & Configuration
// //!
// //! \param  None
// //!
// //! \return None
// //
//*****************************************************************************
static void
BoardInit(void)
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
//! Pong function handling the game
//!
//! \param  None
//!
//! \return None
//! 
//*****************************************************************************

int xCoor = 64;
int yCoor = 64; // origin coordinates

void ball(){
    unsigned char ucDevAddr = 0x18, xOffset = 0x3, yOffset = 0x5, ucRdLen = 1; //hardcoded values from earlier part of lab
    unsigned char XaucRdDataBuf; // to find tilt in y
    unsigned char YaucRdDataBuf; // to find tilt in x

    while(1){
    I2C_IF_Write(ucDevAddr, &xOffset, 1, 0);
    I2C_IF_Read(ucDevAddr, &XaucRdDataBuf, ucRdLen);
    I2C_IF_Write(ucDevAddr, &yOffset, 1, 0);
    I2C_IF_Read(ucDevAddr, &YaucRdDataBuf, ucRdLen);
    fillCircle(xCoor, yCoor, 3, BLUE);

    // signed char gives a range of negative values which can be used to detect if ball is going in the opposite direction
    int xSpeed = (int) ((signed char) YaucRdDataBuf);
    int ySpeed = (int) ((signed char) XaucRdDataBuf); 

    // to give the impression that the ball is moving, filling original position with black
    fillCircle(xCoor, yCoor, 3, BLACK);

    xCoor = xCoor + (int)xSpeed*0.1;

    //checking bounds
    if (xCoor>=123){
        xCoor = 123;
    }
    if (xCoor<=4){
        xCoor = 4;
    }
    yCoor = yCoor + (int)ySpeed*0.1;

    //checking bounds
    if (yCoor>=123){
        yCoor = 123;
    }
    if (yCoor<=4){
        yCoor = 4;
    }

    // filling new positon with ball
    fillCircle(xCoor, yCoor, 3, BLUE);
    }

}
//*****************************************************************************
//
//! Main function handling the I2C example
//!
//! \param  None
//!
//! \return None
//! 
//*****************************************************************************
void main()
 {
    
    //
    // Initialize board configurations
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();
    
    //
    // Configuring UART
    //
    InitTerm();
    ClearTerm();
    
    //
    // I2C Init
    //
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    MAP_PRCMPeripheralReset(PRCM_GSPI);

   //
    // Reset SPI
    //
    MAP_SPIReset(GSPI_BASE);

    //
    // Configure SPI interface
    //
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    //
    // Enable SPI for communication
    //
    MAP_SPIEnable(GSPI_BASE);

    Adafruit_Init();
    fillScreen(BLACK);
    ball();

}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************
