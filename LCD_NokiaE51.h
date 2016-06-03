/* 
 * File:   LCD_NokiaE51.h
 * Author: Krzysiek
 *
 * Created on 24 kwiecie? 2015, 21:23
 */
#include <xc.h>
#include "fonts.h"
#include "hardware.h"
#ifndef LCD_NOKIAE51_H
#define	LCD_NOKIAE51_H

#ifdef	__cplusplus
extern "C" {

#endif

#define LCD_DC  LATAbits.LATA9          // Data = 1 or Command = 0
#define LCD_WRB LATAbits.LATA4          // Latch data (or command) rise edge
#define LCD_RDB LATAbits.LATA10         // Allows to read data (Read data rise edge latch)
#define LCD_CSB LATAbits.LATA8          // Activate system = 0
#define LCD_RST LATAbits.LATA7          // Reset system = 1 (normal = 0)

#define LCD_D0  LATCbits.LATC0
#define LCD_D1  LATCbits.LATC1
#define LCD_D2  LATCbits.LATC2
#define LCD_D3  LATCbits.LATC3
#define LCD_D4  LATCbits.LATC4
#define LCD_D5  LATCbits.LATC5
#define LCD_D6  LATCbits.LATC6
#define LCD_D7  LATCbits.LATC7

#define USE_ALL_PORT 1

#if USE_ALL_PORT == 1
#define LCD_DATA_PORT LATC
#endif
    /* Commends for LCD*/
#define NOP     0x00
#define SWRESET 0x01
#define SLPOUT  0x11
#define DISPON  0x29
#define DISPOFF 0x28
#define CASET   0x2A
#define RASET   0x2B
#define RAMWR   0x2C
#define MADCTR  0x36
#define WRDISBV 0x51

#define BarsSpace 2
#define BuffForBMP  3072

    /*Window details*/
#define colourLine 0x000000
#define colourBackground 0xFFFFFF
    ////////////////////////
/* Function definition*/
    typedef struct {
        unsigned int x_psition;
        unsigned int y_position;
        unsigned int width;
        unsigned long color_frame;
        unsigned long color_inside;
        unsigned long color_text;
        unsigned char paintFlag;
    } typeWindow;

    typedef struct {
        unsigned char second;
        unsigned char minute;
        unsigned char hour;
    } struct_RTC_Time;

    typedef struct {
        unsigned char wday;
        unsigned char day;
        unsigned char month;
        unsigned char year;
    } struct_RTC_Date;


    void LCD_Initialize(void);
    void LCD_PaintColor(unsigned long color);
    void LCD_PaintBlack(void);
    void LCD_PaintWithe(void);
    void LCD_Flash(void);

    void LCD_PutPixel(unsigned int x, unsigned int y, unsigned long color);
    void LCD_PutString(unsigned int x, unsigned int y, unsigned long color, char *chString);
    void LCD_PutStringV(unsigned int x, unsigned int y, unsigned long color, char *chString);
    void LCD_PutInt(unsigned int x, unsigned int y, unsigned long color, unsigned int tmp);
    void LCD_Put_Pictures(unsigned int x, unsigned int y, unsigned int size_x, unsigned int size_y, const char * tmp_pictures);
    void LCD_PutFloat(unsigned int x, unsigned int y, unsigned long color, float tmp);
    void LCD_PutFloatWithNrPl(unsigned int x, unsigned int y, unsigned long color, unsigned char comma, float tmp);
    void LCD_PutFloatV(unsigned int x, unsigned int y, unsigned long color, float tmp);
    void LCD_PaintChart(unsigned int x, unsigned int y, unsigned int width, unsigned int hight,
            unsigned char uchNrBars, float* fSamplesBars);
    void LCD_Line(int X1, int Y1, int X2, int Y2, unsigned long color);
    void LCD_Circle(int cx, int cy, int radius, unsigned long color);
    void LCD_RoundRect(int x, int y, int width, int height, unsigned char radius, unsigned char fill, unsigned long color1, unsigned long color2);
    void LCD_SetCurrentFont(const fontType*);
    void LCD_PutWindow(typeWindow *window, unsigned int tmp);

    void LCD_PutWindowFloat(typeWindow *window, float tmp);
    void LCD_PutWindowWithTitleInt(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, unsigned int tmp);
    void LCD_PutWindowWithTitleFloat(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, float tmp);
    void LCD_PutTemperatureWindow(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, float tmp);
    void LCD_PutTimeWindow(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, struct_RTC_Time *time);
    void LCD_DrawBMP(int x, int y, char * name);
    void LCD_PutTimeWindowInt(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, unsigned int time);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_NOKIAE51_H */

