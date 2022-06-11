#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#define WIDTH 128
#define HEIGHT 128  // SET THIS TO 96 FOR 1.27"!

/*
#if ARDUINO >= 100
 #include "Arduino.h"
 #include "Print.h"
#else
 #include "WProgram.h"
#endif
*/

#define swap(a, b) {int t = a; a = b; b = t; }

// class Adafruit_GFX : public Print {

// public:

//  Adafruit_GFX(int w, int h); // Constructor

  // This MUST be defined by the subclass:
  void drawPixel(int x, int y, unsigned int color);

  // These MAY be overridden by the subclass to provide device-specific
  // optimized code.  Otherwise 'generic' versions are used.


    void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
//    void drawFastVLine(int x, int y, int h, unsigned int color);
//    void drawFastHLine(int x, int y, int w, unsigned int color);
    void drawRect(int x, int y, int w, int h, unsigned int color);
//    void fillRect(int x, int y, int w, int h, unsigned int color);
//    void fillScreen(unsigned int color);
    void invertDisplay(char i);

  // These exist only with Adafruit_GFX (no subclass overrides)
  
    void drawCircle(int x0, int y0, int r, unsigned int color);
    void drawCircleHelper(int x0, int y0, int r, unsigned char cornername, unsigned int color);
    void fillCircle(int x0, int y0, int r, unsigned int color);
    void fillCircleHelper(int x0, int y0, int r, unsigned char cornername, int delta, unsigned int color);
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int color);
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int color);
    void drawRoundRect(int x0, int y0, int w, int h, int radius, unsigned int color);
    void fillRoundRect(int x0, int y0, int w, int h, int radius, unsigned int color);
    void drawBitmap(int x, int y, const unsigned char *bitmap, int w, int h, unsigned int color);
//    void drawBitmap(int x, int y, const unsigned char *bitmap, int w, int h, unsigned int color, unsigned int bg);
    void drawXBitmap(int x, int y, const unsigned char *bitmap, int w, int h, unsigned int color);
    void drawChar(int x, int y, unsigned char c, unsigned int color, unsigned int bg, unsigned char size);
    void setCursor(int x, int y);
//    void setTextColor(unsigned int c);
    void setTextColor(unsigned int c, unsigned int bg);
    void setTextSize(unsigned char s);
    void setTextWrap(char w);
//    void setRotation(unsigned char r);

#if ARDUINO >= 100
  virtual size_t write(unsigned char);
#else
  void   write(unsigned char);
#endif

  int height(void);
  int width(void);

  unsigned char getRotation(void);
/*
  const int
    WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
  int
    _width, _height, // Display w/h as modified by current rotation
    cursor_x, cursor_y;
  unsigned int
    textcolor, textbgcolor;
  unsigned char
    textsize,
    rotation;
  char
    wrap; // If set, 'wrap' text at right edge of display
*/
#endif // _ADAFRUIT_GFX_H
