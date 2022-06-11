// Kunal Pai, Steven To
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"
#include "spi.h"


// Common interface includes
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "test.h"
#include "uart_if.h"
#include "timer_if.h"

#include "pin_mux_config.h"

#define SPI_IF_BIT_RATE  400000
#define TR_BUFF_SIZE     100
#define CONSOLE1         UARTA1_BASE
#define CONSOLE1_PERIPH  PRCM_UARTA1

// 1 = Red, 2 = Orange, 3 = Yellow, 4 = Green, 5 = Blue
unsigned int colors[] = {RED, MAGENTA, YELLOW, GREEN, BLUE};
unsigned int appliedColors[50] = {0};
int colorIndex = 0;
int buttonIndex = 0;
int prevPress;
int colorChange;
int multiPress;
int firstPress;
char button2[] = "abc";
char button3[] = "def";
char button4[] = "ghi";
char button5[] = "jkl";
char button6[] = "mno";
char button7[] = "pqrs";
char button8[] = "tuv";
char button9[] = "wxyz";
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif


int N = 410;                    // block size
volatile long int samples[410];  // buffer to store N samples
int num_samples = 0;          // to check validity of button press
volatile int count = 0;         // samples count
volatile int flag;          // flag set when the samples buffer is full with N samples
volatile int new_dig;       // flag set when inter-digit interval (pause) is detected
long int power_all[7];       // array to store calculated power of 7 frequencies
long int coeff[7] = {31548, 31281, 30951, 30556, 29144, 28361, 27409}; // coeff array calculated using formula outlined in lab report
int newChar = 0;
int delete = 0;
char post;
char prevbutton;
char message[50] = "";
int messageIndex = 0;
int spaceUsed = 0;
int buttonPressed = 1;
int x = 64;
int y = 64;
volatile int res;
int trigger = 0;
char prev = '\0';
char received[50];
int sendFlag, receiveFlag;
int receive;


//*****************************************************************************
//! Board Initialization & Configuration
//*****************************************************************************
long int goertzel (long int coeff);

void UARTIntHandler(void)
{
    UARTIntClear(CONSOLE1, UART_INT_RX | UART_INT_RT | UART_INT_TX);

    unsigned long intStatus = UARTIntStatus(CONSOLE1, true);

    while (UARTCharsAvail(CONSOLE1)) {
        unsigned char character = MAP_UARTCharGet(CONSOLE1);
        printf("%c\n", character);
        if (character == '\0'){ // null terminator, string has been processed
            receiveFlag = 1;
            fillRect(0, 64, 128, 64, BLUE);
            setCursor(0, 64);
            printf("%d\n", received);
            Outstr(received);
            receiveFlag = 0;
        }
        else { // characters available in message
            received[receive] = character;
            receive++;
        }

    }
}


void UartInit() {
  MAP_UARTConfigSetExpClk(
      UARTA1_BASE, MAP_PRCMPeripheralClockGet(PRCM_UARTA1), UART_BAUD_RATE,
      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  MAP_UARTIntRegister(UARTA1_BASE, UARTIntHandler);
  MAP_UARTIntEnable(UARTA1_BASE,  UART_INT_RX | UART_INT_RT | UART_INT_TX);

  unsigned long uartStatus = UARTIntStatus(UARTA1_BASE, false);
  UARTIntClear(UARTA1_BASE, uartStatus);

  MAP_UARTDMAEnable(UARTA1_BASE, UART_DMA_TX);
}

unsigned short transferSignal(void)
{
    unsigned char data[2];
    GPIOPinWrite(GPIOA2_BASE, 0x40, 0x00); // CS
    MAP_SPITransfer(GSPI_BASE, 0, data, 0x2, SPI_CS_ENABLE|SPI_CS_DISABLE); // SPI
    GPIOPinWrite(GPIOA2_BASE, 0x40, 0x40); // CS
    return (data[0] << 5) | ((0xf8 & data[1]) >> 3);
}

static void
BoardInit(void)
{
#ifndef USE_TIRTOS
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

long int goertzel (long int coeff)
//---------------------------------------------------------------//
{
//initialize variables to be used in the function
  long long Q, Q_prev, Q_prev2, i;
  long long prod1, prod2, prod3, power;

  Q_prev = 0;           //set delay element1 Q_prev as zero
  Q_prev2 = 0;          //set delay element2 Q_prev2 as zero
  power = 0;            //set power as zero

  for (i = 0; i < 410; i++)   // loop N times and calculate Q, Q_prev, Q_prev2 at each iteration
    {
      Q = (samples[i]) + ((coeff * Q_prev) >> 14) - (Q_prev2);   // >>14 used as the coeff was used in Q15 format
      Q_prev2 = Q_prev;     // shuffle delay elements
      Q_prev = Q;
    }

  //calculate the three products used to calculate power
  prod1 = ((long long) Q_prev * Q_prev);
  prod2 = ((long long) Q_prev2 * Q_prev2);
  prod3 = ((long long) Q_prev * coeff) >> 14;
  prod3 = (prod3 * Q_prev2);

  power = ((prod1 + prod2 - prod3)) >> 8;   //calculate power using the three products and scale the result down

  return (long) power;
}

char post_test(void)
{
    //initialize variables to be used in the function
    int i, row, col, max_power;

    char row_col[4][3] =        // array with the order of the digits in the DTMF system
    {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}
    };
    // find the maximum power in the row frequencies and the row number

    max_power = 0;              //initialize max_power=0

    for (i = 0; i < 4; i++)     //loop 4 times from 0>3 (the indecies of the rows)
    {
      //printf("power_all: %d\n", power_all[i]);
      if (power_all[i] > max_power) //if power of the current row frequency > max_power
      {
        max_power = power_all[i];   //set max_power as the current row frequency
        row = i;        //update row number
      }
    }


  // find the maximum power in the column frequencies and the column number

  max_power = 0;        //initialize max_power=0

  for (i = 4; i < 7; i++)   //loop 4 times from 4>6 (the indecies of the columns)
    {
      if (power_all[i] > max_power) //if power of the current column frequency > max_power
      {
        max_power = power_all[i];   //set max_power as the current column frequency
        col = i;        //update column number
      }
    }



  if (power_all[col] > 100000000 && new_dig == 1) { // bound discovered through experimentation
      char ret = row_col[row][col - 4];
      new_dig = 0;      // set new_dig to 0 to avoid displaying the same digit again.

      return ret;
  } else {
      new_dig = 1;
  }

  return 'x';
}

static void TimerA1IntHandler(void) // every 5000 ticks or 0.625 s to transfer a sample from ADC and fill up sample array
{
    unsigned long ulStatus;
    ulStatus = MAP_TimerIntStatus(TIMERA1_BASE, true);
    MAP_TimerIntClear(TIMERA1_BASE, ulStatus);
    long val = ((signed long)transferSignal())-372;

    samples[count] = val;
    count++;

    if (count >= 410)
        res = 1;
}

static void TimerA0IntHandler(void) // timeout so that new character can be accepted and old char is locked in for texting
{
    unsigned long ulStatus;
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);

    newChar = 1;
}

char* decodeInput(void) { // decoding input and doing things as per the buttons, also checking for multiple presses of the same button for texting functionality
    if (post == '*' || post == '#' || post == '0' || post == '1')
        multiPress = 0;
    else
        multiPress = 1;


    if (prevbutton != post && multiPress && firstPress == 0 && prevPress) {
        messageIndex++;
        buttonIndex = 0;
    }

    if (post == '1'){
        colorChange = 1;
        prevPress = 0;
        colorIndex++;
        printf("Color Changed\n");
    }

    if (post == '2') {
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button2[buttonIndex%3];
        buttonIndex++;

    }

    if (post == '3'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button3[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '4'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button4[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '5'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button5[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '6'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button6[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '7'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button7[buttonIndex%3];
        buttonIndex++;
        //printf("7\n");
    }

    if (post == '8'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button8[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '9'){
        prevPress = 1;
        buttonPressed = 1;
        message[messageIndex] = button9[buttonIndex%3];
        buttonIndex++;
    }

    if (post == '*') { // mute
        prevPress = 0;

        delete = 1;

        messageIndex--;
        message[messageIndex] = '\0';
    }

    if (post == '0') {
        prevPress = 0;
        strcat(message, " ");
        messageIndex++;
        spaceUsed = 1;
        printf("Space Added\n");
    }

    if (post == '#') { // last
        prevPress = 0;
        message[messageIndex] = '\0';
        sendFlag = 1;
    }

    appliedColors[messageIndex] = colors[colorIndex % 5];


    prevbutton = post;
    firstPress = 0;

    return NULL;
}

int main() {
    unsigned long ulStatus;

    res = 0;

    // Initializations
    BoardInit();
    PinMuxConfig();
    //SPI INIT
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_GSPI);
    MAP_SPIReset(GSPI_BASE);
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                           (SPI_SW_CTRL_CS |
                           SPI_4PIN_MODE |
                           SPI_TURBO_OFF |
                           SPI_CS_ACTIVEHIGH |
                           SPI_WL_8));
    MAP_SPIEnable(GSPI_BASE);

    Adafruit_Init();
    setCursor(0,0);
    setTextSize(2);
    fillScreen(BLUE);
    setTextColor(colors[colorIndex % 5], BLACK);

    // Sampling timer
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA1);
    MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_PERIODIC);

    // Timeout timer
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_ONE_SHOT);


    // Set up interrupt handler for both timers
    MAP_TimerIntRegister(TIMERA1_BASE, TIMER_A, TimerA1IntHandler);
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_A, TimerA0IntHandler);
    ulStatus = MAP_TimerIntStatus(TIMERA1_BASE, false);
    MAP_TimerIntClear(TIMERA1_BASE, ulStatus);
    ulStatus = MAP_TimerIntStatus(TIMERA0_BASE, false);
    MAP_TimerIntClear(TIMERA0_BASE, ulStatus);


    // Set time for both timers
    MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, 5000);
    MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 100050000);

    // Enable sampling timer
    MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
    MAP_TimerEnable(TIMERA1_BASE, TIMER_A);

    UartInit();

    while (1)
    {
        // Sample analysis in main
        if (res==1)
        {
            // disabling sample timer
            MAP_TimerIntDisable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
            MAP_TimerDisable(TIMERA1_BASE, TIMER_A); 

            res = 0;
            count = 0;

            int i;
            for (i = 0; i < 7; i++){ // putting all values of coeff into the goertzel algorithm to compute power
                power_all[i] = goertzel(coeff[i]);
            }



            // Decoding
            post = post_test();

            if (post == prev)
                num_samples++; // for sample confidence

            prev = post;

            // Confirm button press
            if (newChar == 1) {
                newChar = 0;
                printf("Character Locked In\n");
                messageIndex++;
                buttonIndex = 0;
                MAP_TimerIntDisable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
                MAP_TimerDisable(TIMERA0_BASE, TIMER_A); // disabling timeout timer
            }

            // Valid button press
            if (num_samples > 10 && post != 'x') {
                MAP_TimerIntDisable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
                MAP_TimerDisable(TIMERA0_BASE, TIMER_A); // disabling timeout timer
                num_samples = 0;

                printf("post: %c\n", post);
                decodeInput();

                // No need to print anything yet
                if (spaceUsed || colorChange) {
                    spaceUsed = 0;
                    colorChange = 0;
                    MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, 5000);
                    MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
                    MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
                    continue;
                }


                if (delete) {
                    delete = 0;
                    fillScreen(BLACK);
                    setCursor(0,0);

                    int d;
                    for (d = 0; d < messageIndex + 1; d++) {
                        setTextColor(appliedColors[d], BLACK); // accouting for color changes
                        char letter = message[d];
                        Outstr(&letter);
                    }

                    printf("Character Deleted\n");
                    MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, 5000);
                    MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
                    MAP_TimerEnable(TIMERA1_BASE, TIMER_A);
                    continue;
                }

                fillScreen(BLACK);
                setCursor(0,0);

                int d;
                for (d = 0; d < messageIndex + 1; d++) {
                    setTextColor(appliedColors[d], BLACK);
                    char letter = message[d];
                    Outstr(&letter);
                }


                if (sendFlag == 1){
                    for (i = 0; i < messageIndex + 1; i++)
                    {
                        MAP_UARTCharPut(CONSOLE1, message[i]); //communicating via UART1
                        printf("%c", message[i]);
                    }
                    fillRect(0, 0, 128, 64, BLACK);
                    memset(message, '\0', 128);
                    sendFlag = 0;
                }

                MAP_TimerLoadSet(TIMERA0_BASE, TIMER_A, 100050000);
                MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMA_TIMEOUT);
                MAP_TimerEnable(TIMERA0_BASE, TIMER_A); // reenabled timer timeout
            }


            MAP_TimerLoadSet(TIMERA1_BASE, TIMER_A, 5000);
            MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT);
            MAP_TimerEnable(TIMERA1_BASE, TIMER_A); // reenable sample counter

        }
    }

    return 0;
}