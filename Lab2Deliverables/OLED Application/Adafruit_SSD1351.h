/*************************************************** 
  This is a library for the 1.5" & 1.27" 16-bit Color OLEDs 
  with SSD1331 driver chip

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1431
  ------> http://www.adafruit.com/products/1673

  These displays use SPI to communicate, 4 or 5 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#define SSD1351WIDTH 128
#define SSD1351HEIGHT 128  // SET THIS TO 96 FOR 1.27"!

//#define swap(a, b) { unsigned int t = a; a = b; b = t; }

/*
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
*/

#ifdef __SAM3X8E__
    typedef volatile RwReg PortReg;
    typedef unsigned int32_t PortMask;
#else
    typedef volatile unsigned char PortReg;
    typedef unsigned char PortMask;
#endif

// Select one of these defines to set the pixel color order
#define SSD1351_COLORORDER_RGB
// #define SSD1351_COLORORDER_BGR

#if defined SSD1351_COLORORDER_RGB && defined SSD1351_COLORORDER_BGR
  #error "RGB and BGR can not both be defined for SSD1351_COLORODER."
#endif

// Timing Delays
#define SSD1351_DELAYS_HWFILL	    (3)
#define SSD1351_DELAYS_HWLINE       (1)

// SSD1351 Commands
#define SSD1351_CMD_SETCOLUMN 		0x15
#define SSD1351_CMD_SETROW    		0x75
#define SSD1351_CMD_WRITERAM   		0x5C
#define SSD1351_CMD_READRAM   		0x5D
#define SSD1351_CMD_SETREMAP 		0xA0
#define SSD1351_CMD_STARTLINE 		0xA1
#define SSD1351_CMD_DISPLAYOFFSET 	0xA2
#define SSD1351_CMD_DISPLAYALLOFF 	0xA4
#define SSD1351_CMD_DISPLAYALLON  	0xA5
#define SSD1351_CMD_NORMALDISPLAY 	0xA6
#define SSD1351_CMD_INVERTDISPLAY 	0xA7
#define SSD1351_CMD_FUNCTIONSELECT 	0xAB
#define SSD1351_CMD_DISPLAYOFF 		0xAE
#define SSD1351_CMD_DISPLAYON     	0xAF
#define SSD1351_CMD_PRECHARGE 		0xB1
#define SSD1351_CMD_DISPLAYENHANCE	0xB2
#define SSD1351_CMD_CLOCKDIV 		0xB3
#define SSD1351_CMD_SETVSL 		0xB4
#define SSD1351_CMD_SETGPIO 		0xB5
#define SSD1351_CMD_PRECHARGE2 		0xB6
#define SSD1351_CMD_SETGRAY 		0xB8
#define SSD1351_CMD_USELUT 		0xB9
#define SSD1351_CMD_PRECHARGELEVEL 	0xBB
#define SSD1351_CMD_VCOMH 		0xBE
#define SSD1351_CMD_CONTRASTABC		0xC1
#define SSD1351_CMD_CONTRASTMASTER	0xC7
#define SSD1351_CMD_MUXRATIO            0xCA
#define SSD1351_CMD_COMMANDLOCK         0xFD
#define SSD1351_CMD_HORIZSCROLL         0x96
#define SSD1351_CMD_STOPSCROLL          0x9E
#define SSD1351_CMD_STARTSCROLL         0x9F


/*
class Adafruit_SSD1351  : public virtual Adafruit_GFX {
 public:
  Adafruit_SSD1351(unsigned char CS, unsigned char RS, unsigned char SID, unsigned char SCLK, unsigned char RST);
  Adafruit_SSD1351(unsigned char CS, unsigned char RS, unsigned char RST);

  unsigned int Color565(unsigned char r, unsigned char g, unsigned char b);
*/

  void Adafruit_Init(void);
	void Outstr (char * str);
	
  // drawing primitives!
  void drawPixel(int x, int y, unsigned int color);
  void fillRect(unsigned int x0, unsigned int y0, unsigned int w, unsigned int h, unsigned int color);
  void drawFastHLine(int x, int y, int w, unsigned int color);
  void drawFastVLine(int x, int y, int h, unsigned int color);
  void fillScreen(unsigned int fillcolor);

  void invert(char);
  // commands
  void begin(void);
  void goTo(int x, int y);

  void reset(void);

  /* low level */

  void writeData(unsigned char d);
  void writeCommand(unsigned char c);


  void writeData_unsafe(unsigned int d);

  void setWriteDir(void);
  void write8(unsigned char d);

/*
 private:
  void spiwrite(unsigned char);

  unsigned char _cs, _rs, _rst, _sid, _sclk;
  PortReg *csport, *rsport, *sidport, *sclkport;
  PortMask cspinmask, rspinmask, sidpinmask, sclkpinmask;
*/
