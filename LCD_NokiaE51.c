#define FCY 40000000UL
#include "LCD_NokiaE51.h"
#include <xc.h>
#include <libpic30.h>
#include "fonts.h"
#include "i2c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

fontType CurrFontType;

const unsigned char filebuffer[ BuffForBMP ];
static inline void LCD_WriteByte(unsigned char);
static inline void LCD_WriteCommand(unsigned char);
static inline void LCD_WriteData(unsigned char);

static void lcd_active_window(int x1, int y1, int x2, int y2) {
    LCD_WriteCommand(CASET);
    LCD_WriteData((unsigned char) (x1 >> 8));
    LCD_WriteData((unsigned char) x1);
    LCD_WriteData((unsigned char) (x2 >> 8));
    LCD_WriteData((unsigned char) (x2));
    LCD_WriteCommand(RASET);
    LCD_WriteData((unsigned char) (y1 >> 8));
    LCD_WriteData((unsigned char) y1);
    LCD_WriteData((unsigned char) (y2 >> 8));
    LCD_WriteData((unsigned char) (y2));
}

static void lcd_put_line(int x1, int y1, int x2, int y2, unsigned long color) {
    lcd_active_window(x1, y1, x2, y2);
    LCD_WriteCommand(RAMWR);
    unsigned int j, i = (x2 - x1 + 1)*(y2 - y1 + 1);
    for (j = 0; j < i; ++j) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
    }
}

void tft_round_rect1(int x, int y, int width, int height, unsigned char radius, unsigned char fill, unsigned long color) {
    int tSwitch;
    int x1 = 0, y1 = radius;
    tSwitch = 3 - 2 * radius;
    while (x1 <= y1) {
        if (fill) {
            LCD_Line(x + radius - x1, y + radius - y1, x + width - radius + x1, y + radius - y1, color);
            LCD_Line(x + radius - y1, y + radius - x1, x + width - radius + y1, y + radius - x1, color);
            LCD_Line(x + width - radius + x1, y + height - radius + y1, x + radius - x1, y + height - radius + y1, color);
            LCD_Line(x + width - radius + y1, y + height - radius + x1, x + radius - y1, y + height - radius + x1, color);
        } else {
            LCD_PutPixel(x + radius - x1, y + radius - y1, color);
            LCD_PutPixel(x + radius - y1, y + radius - x1, color);

            LCD_PutPixel(x + width - radius + x1, y + radius - y1, color);
            LCD_PutPixel(x + width - radius + y1, y + radius - x1, color);

            LCD_PutPixel(x + width - radius + x1, y + height - radius + y1, color);
            LCD_PutPixel(x + width - radius + y1, y + height - radius + x1, color);

            LCD_PutPixel(x + radius - x1, y + height - radius + y1, color);
            LCD_PutPixel(x + radius - y1, y + height - radius + x1, color);
        }

        if (tSwitch < 0) {
            tSwitch += (4 * x1 + 6);
        } else {
            tSwitch += (4 * (x1 - y1) + 10);
            y1--;
        }
        x1++;
    }

    if (fill) {
        unsigned int i;
        for (i = 0; i < height - (2 * radius); i++) lcd_put_line(x, y + radius + i, x + width, y + radius + i, color);
    }

    lcd_put_line(x + radius, y, x + width - radius, y, color);
    lcd_put_line(x + radius, y + height, x + width - radius, y + height, color);

    lcd_put_line(x, y + radius, x, y + height - radius, color);
    lcd_put_line(x + width, y + radius, x + width, y + height - radius, color);
}

void LCD_RoundRect(int x, int y, int width, int height, unsigned char radius, unsigned char fill, unsigned long color1, unsigned long color2) {

    if (fill) {
        tft_round_rect1(x, y, width, height, radius, 1, color1);
        tft_round_rect1(x + 1, y + 1, width - 2, height - 2, radius, 1, color2);
    } else {
        tft_round_rect1(x, y, width, height, radius, 0, color1);
    }
}

void LCD_Circle(int cx, int cy, int radius, unsigned long color) {
    int x, y, xchange, ychange, radiusError;
    x = radius;
    y = 0;
    xchange = 1 - 2 * radius;
    ychange = 1;
    radiusError = 0;
    while (x >= y) {
        LCD_PutPixel(cx + x, cy + y, color);
        LCD_PutPixel(cx - x, cy + y, color);
        LCD_PutPixel(cx - x, cy - y, color);
        LCD_PutPixel(cx + x, cy - y, color);
        LCD_PutPixel(cx + y, cy + x, color);
        LCD_PutPixel(cx - y, cy + x, color);
        LCD_PutPixel(cx - y, cy - x, color);
        LCD_PutPixel(cx + y, cy - x, color);
        ++y;
        radiusError += ychange;
        ychange += 2;
        if (2 * radiusError + xchange > 0) {
            x--;
            radiusError += xchange;
            xchange += 2;
        }
    }
}

void LCD_Line(int X1, int Y1, int X2, int Y2, unsigned long color) {
    int CurrentX, CurrentY, Xinc, Yinc, Dx, Dy, TwoDx, TwoDy, TwoDxAccumulatedError, TwoDyAccumulatedError;
    Dx = X2 - X1; // calculating the horizontal component
    Dy = Y2 - Y1; // calculating the vertical component
    TwoDx = Dx + Dx;
    TwoDy = Dy + Dy;
    CurrentX = X1;
    CurrentY = Y1;
    Xinc = 1; // horizontal step increse (negative step)
    Yinc = 1; // vertical step increse
    if (Dx < 0) // if is negative
    {
        Xinc = -1; // will be shifted back
        Dx = -Dx;
        TwoDx = -TwoDx;
    }
    if (Dy < 0) // these same
    {
        Yinc = -1;
        Dy = -Dy;
        TwoDy = -TwoDy;
    }
    LCD_PutPixel(X1, Y1, color); // draw first pixel
    if ((Dx != 0) || (Dy != 0)) // line has more than one pixel hight or width
    { // horizontal component is smaller or equal than vertical component
        if (Dy <= Dx) // if yes, go x
        {
            TwoDxAccumulatedError = 0;
            do {
                CurrentX += Xinc; // add a step to the current position
                TwoDxAccumulatedError += TwoDy; // add double vertical component
                if (TwoDxAccumulatedError > Dx) {
                    CurrentY += Yinc; // increse present position
                    TwoDxAccumulatedError -= TwoDx;
                }
                LCD_PutPixel(CurrentX, CurrentY, color); // next pixel
            } while (CurrentX != X2);
        } else {
            TwoDyAccumulatedError = 0;
            do {
                CurrentY += Yinc;
                TwoDyAccumulatedError += TwoDx;
                if (TwoDyAccumulatedError > Dy) {
                    CurrentX += Xinc;
                    TwoDyAccumulatedError -= TwoDy;
                }
                LCD_PutPixel(CurrentX, CurrentY, color);
            } while (CurrentY != Y2);
        }
    }
}

void LCD_PaintChart(unsigned int x, unsigned int y, unsigned int width, unsigned int hight,
        unsigned char uchNrBars, float* fSamplesBars) {

    unsigned int i, j, k;
    char tmpArrow[] = {0x18, 0x3C, 0x7E, 0xDB, 0x99, 0x18, 0x18, 0x18,};
    for (j = 0; j < 8; ++j) {
        for (i = 0; i < 8; ++i) {
            if ((tmpArrow[j] >> i)&0x01)
                LCD_PutPixel(x + i, y + j, 0);
        }
    }
    LCD_WriteCommand(RASET);
    LCD_WriteData((unsigned char) ((y + 8) >> 8));
    LCD_WriteData((unsigned char) (y + 8));
    LCD_WriteData((unsigned char) (((y + hight) - 4) >> 8));
    LCD_WriteData((unsigned char) ((y + hight) - 4));
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (x + 3));
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (x + 4));
    LCD_WriteCommand(RAMWR);
    j = 2 * (hight);
    LCD_WriteData(0x00);
    for (i = 0; i < j; ++i) {
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
    }
    LCD_WriteCommand(RASET);
    LCD_WriteData((unsigned char) (((y + hight) - 5) >> 8));
    LCD_WriteData((unsigned char) ((y + hight) - 5));
    LCD_WriteData((unsigned char) (((y + hight) - 4) >> 8));
    LCD_WriteData((unsigned char) ((y + hight) - 4));
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (x + 5));
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (x + width - 8));
    LCD_WriteCommand(RAMWR);
    j = 2 * (width - 12);
    LCD_WriteData(0x00);
    for (i = 0; i < j; ++i) {
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
    }
    for (j = 0; j < 8; ++j) {
        for (i = 0; i < 8; ++i) {
            if ((tmpArrow[j] >> i)&0x01)
                LCD_PutPixel((x + width) - j, (y + hight - 8) + i, 0);
        }
    }
    unsigned int uiTmpBarsWidth, uiTmpBarHigh, uiTmpNrHighestSamples = 0;
    uiTmpBarsWidth = ((width - 20) - BarsSpace * uchNrBars) / uchNrBars;
    for (i = 1; i < uchNrBars; ++i) if (fSamplesBars[i] > fSamplesBars[uiTmpNrHighestSamples]) uiTmpNrHighestSamples = i;
    float tmp;
    for (i = 0; i < uchNrBars; ++i) {
        tmp = fSamplesBars[i] / fSamplesBars[uiTmpNrHighestSamples];
        uiTmpBarHigh = (unsigned int) (tmp * (hight - 15));
        LCD_WriteCommand(RASET);
        LCD_WriteData((unsigned char) ((y + hight - 6 - uiTmpBarHigh) >> 8));
        LCD_WriteData((unsigned char) (y + hight - 6 - uiTmpBarHigh));
        LCD_WriteData((unsigned char) (((y + hight) - 6) >> 8));
        LCD_WriteData((unsigned char) ((y + hight) - 6));
        LCD_WriteCommand(CASET);
        LCD_WriteData(0);
        LCD_WriteData((unsigned char) (x + 10 + (i * BarsSpace)+(i * uiTmpBarsWidth)));
        LCD_WriteData(0);
        LCD_WriteData((unsigned char) (x + 10 + uiTmpBarsWidth + (i * BarsSpace)+(i * uiTmpBarsWidth)));
        LCD_WriteCommand(RAMWR);
        LCD_WriteData(0);
        k = (uiTmpBarHigh + 1)*(1 + uiTmpBarsWidth);
        for (j = 0; j < k; ++j) {
            LCD_WRB = 0;            LCD_WRB = 1;
            LCD_WRB = 0;            LCD_WRB = 1;
            LCD_WRB = 0;            LCD_WRB = 1;
        }
    }
}

void LCD_PutFloat(unsigned int x, unsigned int y, unsigned long color, float tmp) {
    int iTmp_cal = 0;
    unsigned int i, p;
    iTmp_cal = (int) tmp;
    tmp -= iTmp_cal;
    float utmp = (tmp > 0) ? tmp : -tmp;
    char string_frac[6];
    for (i = 0; i < 5; ++i) {
        utmp *= 10;
        p = (unsigned int) utmp;
        utmp -= p;
        string_frac[i] = p + 48;
    }
    string_frac[i] = 0;
    char string[17];
    itoa(string, iTmp_cal, 10);
    strcat(string, ".");
    strcat(string, string_frac);
    LCD_PutString(x, y, color, string);
}

void LCD_PutFloatWithNrPl(unsigned int x, unsigned int y, unsigned long color, unsigned char comma, float tmp) {
    int iTmp_cal = 0;
    unsigned int i, p;
    iTmp_cal = (int) tmp;
    tmp -= iTmp_cal;
    float utmp = (tmp > 0) ? tmp : -tmp;
    char string_frac[6];
    for (i = 0; i < comma; ++i) {
        utmp *= 10;
        p = (unsigned int) utmp;
        utmp -= p;
        string_frac[i] = p + 48;
    }
    string_frac[i] = 0;
    char string[17];
    itoa(string, iTmp_cal, 10);
    strcat(string, ".");
    strcat(string, string_frac);
    LCD_PutString(x, y, color, string);
}

void LCD_PutInt(unsigned int x, unsigned int y, unsigned long color, unsigned int tmp) {
    char string[17];
    itoa(string, tmp, 10);
    LCD_PutString(x, y, color, string);
}

void LCD_PutIntV(unsigned int x, unsigned int y, unsigned long color, unsigned int tmp) {
    char string[17];
    itoa(string, tmp, 10);
    LCD_PutStringV(x, y, color, string);
}

void LCD_PutFloatV(unsigned int x, unsigned int y, unsigned long color, float tmp) {
    int iTmp_cal = 0;
    unsigned int i, p;
    iTmp_cal = (int) tmp;
    tmp -= iTmp_cal;
    float utmp = (tmp > 0) ? tmp : -tmp;
    char string_frac[6];
    for (i = 0; i < 5; ++i) {
        utmp *= 10;
        p = (unsigned int) utmp;
        utmp -= p;
        string_frac[i] = p + 48;
    }
    string_frac[i] = 0;
    char string[17];
    itoa(string, iTmp_cal, 10);
    strcat(string, ".");
    strcat(string, string_frac);
    LCD_PutStringV(x, y, color, string);
}

void LCD_Put_Pictures(unsigned int x, unsigned int y, unsigned int size_x, unsigned int size_y, const char * tmp_pictures) {
    LCD_WriteCommand(CASET);
    LCD_WriteData((unsigned char) (x >> 8));
    LCD_WriteData((unsigned char) x);
    LCD_WriteData((unsigned char) ((x + size_x - 1) >> 8));
    LCD_WriteData((unsigned char) (x + size_x - 1));
    LCD_WriteCommand(RASET);
    LCD_WriteData((unsigned char) (y >> 8));
    LCD_WriteData((unsigned char) y);
    LCD_WriteData((unsigned char) ((y + size_y - 1) >> 8));
    LCD_WriteData((unsigned char) (y + size_y - 1));
    LCD_WriteCommand(RAMWR);
    unsigned long j, i = size_x * size_y * 3;
    for (j = 0; j < i; ++j) {
        LCD_WriteData(tmp_pictures[j]);
    }
}

void LCD_PutPixel(unsigned int x, unsigned int y, unsigned long color) {
    LCD_WriteCommand(CASET);
    LCD_WriteData((unsigned char) (x >> 8));
    LCD_WriteData((unsigned char) x);
    LCD_WriteData((unsigned char) (x >> 8));
    LCD_WriteData((unsigned char) x);
    LCD_WriteCommand(RASET);
    LCD_WriteData((unsigned char) (y >> 8));
    LCD_WriteData((unsigned char) y);
    LCD_WriteData((unsigned char) (y >> 8));
    LCD_WriteData((unsigned char) y);
    LCD_WriteCommand(RAMWR);
    LCD_WriteData((unsigned char) color);
    LCD_WriteData((unsigned char) (color >> 8));
    LCD_WriteData((unsigned char) (color >> 16));
}

void LCD_PaintBlack(void) {
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (319 >> 8));
    LCD_WriteData((unsigned char) 319);
    LCD_WriteCommand(RASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(239);
    LCD_WriteCommand(RAMWR);
    unsigned int i, l = 80UL * 320UL;
    LCD_WriteData(0x00);
    for (i = 0; i < l; ++i) {
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
    }
}

void LCD_PaintWithe(void) {
    unsigned int i, l = 80UL * 320UL;
    LCD_WriteCommand(RASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    //    LCD_WriteData((unsigned char)(319>>8));
    //    LCD_WriteData((unsigned char)319);
    LCD_WriteData(0);
    LCD_WriteData(239);
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (319 >> 8));
    LCD_WriteData((unsigned char) 319);
    //    LCD_WriteData(0);
    //    LCD_WriteData(239);
    LCD_WriteCommand(RAMWR);
    LCD_WriteData(0xFF);
    for (i = 0; i < l; ++i) {
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
        LCD_WRB = 0;        LCD_WRB = 1;
    }
}

void LCD_Flash(void) {
    LCD_WriteCommand(RASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(239);
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (319 >> 8));
    LCD_WriteData((unsigned char) 319);
    unsigned long i = 1, j = 320UL * 240UL;
    LCD_WriteCommand(RAMWR);
    while (i == 1) {
        for (i = 0; i < j; ++i) {
            LCD_WriteData(0xFF);
            LCD_WriteData(0x00);
            LCD_WriteData(0x00);
        }
        for (i = 0; i < j; ++i) {
            LCD_WriteData(0x00);
            LCD_WriteData(0xFF);
            LCD_WriteData(0x00);
        }
        for (i = 0; i < j; ++i) {
            LCD_WriteData(0x00);
            LCD_WriteData(0x00);
            LCD_WriteData(0xFF);
        }
    }
}

void LCD_PaintColor(unsigned long color) {
    LCD_WriteCommand(RASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData(239);
    LCD_WriteCommand(CASET);
    LCD_WriteData(0);
    LCD_WriteData(0);
    LCD_WriteData((unsigned char) (319 >> 8));
    LCD_WriteData((unsigned char) 319);
    unsigned long i, j = 240UL * 320UL;
    LCD_WriteCommand(RAMWR);
    for (i = 0; i < j; ++i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
    }
}

void LCD_HardwareReset() {
    LCD_RST = 0;
    __delay_us(100);
    LCD_RST = 1;
    __delay_ms(120); // delay 120 ms for power stabilization
}

void LCD_Initialize() {
    __delay_ms(250); // optional wait for stabilization lcd power
    LCD_HardwareReset();
    LCD_WriteCommand(SWRESET); // software reset
    __delay_ms(5);
    LCD_WriteCommand(SLPOUT); // Turn off sleep mode
    __delay_ms(5);
    LCD_WriteCommand(DISPON); // Display on
    LCD_WriteCommand(MADCTR);
    LCD_WriteData(0b01101000);
}

inline void LCD_WriteCommand(unsigned char cmd) {
    LCD_CSB = 0; // choose LCD
    LCD_DC = 0; // Write Command
    LCD_RDB = 1; // Allows write data
    LCD_WRB = 0; // low on WRB line
    LCD_WriteByte(cmd);
    LCD_WRB = 1; // Latch data
}

inline void LCD_WriteData(unsigned char data) {
    LCD_CSB = 0;
    LCD_DC = 1;
    LCD_RDB = 1;
    LCD_WRB = 0;
    LCD_WriteByte(data);
    LCD_WRB = 1;
};

static inline void LCD_WriteByte(unsigned char tmp) {
#if USE_ALL_PORT == 1
    LCD_DATA_PORT = tmp;
#else
    //    LATB &= 0x0E2F;
    if (tmp & 0x80) LCD_D7 = 1;
    else LCD_D7 = 0;
    if (tmp & 0x40) LCD_D6 = 1;
    else LCD_D6 = 0;
    if (tmp & 0x20) LCD_D5 = 1;
    else LCD_D5 = 0;
    if (tmp & 0x10) LCD_D4 = 1;
    else LCD_D4 = 0;
    if (tmp & 0x08) LCD_D3 = 1;
    else LCD_D3 = 0;
    if (tmp & 0x04) LCD_D2 = 1;
    else LCD_D2 = 0;
    if (tmp & 0x02) LCD_D1 = 1;
    else LCD_D1 = 0;
    if (tmp & 0x01) LCD_D0 = 1;
    else LCD_D0 = 0;
#endif
}

void LCD_SetCurrentFont(const fontType *fTFont) {
    CurrFontType.FirstCharCode = fTFont->FirstCharCode;
    CurrFontType.Height = fTFont->Height;
    CurrFontType.Interspace = fTFont->Interspace;
    CurrFontType.SpaceWidth = fTFont->SpaceWidth;
    CurrFontType.CharInfo = fTFont->CharInfo;
    CurrFontType.Bitmap = fTFont->Bitmap;
}

static unsigned char lcdputchar(int x, int y, char tmp, unsigned long color) {
    unsigned char uchHeight, uchWidtch, uchFirstChar, tmpData, i = 0, j, k, l, z = 8, r = 0;
    unsigned int uiOffset;
    uchHeight = CurrFontType.Height;
    uchFirstChar = CurrFontType.FirstCharCode;
    uchFirstChar = tmp - uchFirstChar;
    uchWidtch = CurrFontType.CharInfo[uchFirstChar].Width;
    uiOffset = CurrFontType.CharInfo[uchFirstChar].Offset;
    if (uchHeight % 8) {
        uchHeight += 8;
        r = 1;
    }
    while (i < uchHeight / 8) {
        for (j = 0; j < uchWidtch; ++j) {
            tmpData = CurrFontType.Bitmap[uiOffset + j + i * uchWidtch];
            l = 1;
            for (k = 0; k < z; ++k) {
                if (tmpData & l) LCD_PutPixel(x + j, y + k + i * 8, color);
                l <<= 1;
            }
        }
        ++i;
        if (r) if (!((i + 1) < uchHeight / 8)) z = uchHeight % 8;
    }
    return uchWidtch;
}

void LCD_PutString(unsigned int x, unsigned int y, unsigned long color, char *chString) {
    unsigned char uchInterspace, uchSpace, i = 0;
    unsigned int uiTmpX = x;
    uchInterspace = CurrFontType.Interspace;
    uchSpace = CurrFontType.SpaceWidth;
    while (chString[i]) {
        if (chString[i] == ' ') uiTmpX += uchSpace;
        else uiTmpX = uiTmpX + uchInterspace + lcdputchar(uiTmpX, y, chString[i], color);
        ++i;
    }
}

void LCD_PutStringV(unsigned int x, unsigned int y, unsigned long color, char *chString) {
    LCD_WriteCommand(MADCTR);
    LCD_WriteData(0b10100000);
    unsigned char uchInterspace, uchSpace, i = 0;
    unsigned int uiTmpX = x;
    uchInterspace = CurrFontType.Interspace;
    uchSpace = CurrFontType.SpaceWidth;
    while (chString[i]) {
        if (*chString == ' ') uiTmpX += uchSpace;
        else uiTmpX = uiTmpX + uchInterspace + lcdputchar(uiTmpX, y, chString[i], color);
        ++i;
    }
    LCD_WriteCommand(MADCTR);
    LCD_WriteData(0);
}

void LCD_PutWindow(typeWindow *window, unsigned int tmp) {
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                CurrFontType.Height + 10, 5, 1, window->color_frame, window->color_inside);
        window->paintFlag = 1;
    }
    lcd_active_window(window->x_psition, window->y_position, window->x_psition + window->width,
            window->y_position + CurrFontType.Height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    LCD_PutInt(window->x_psition, window->y_position, window->color_text, tmp);
}

void LCD_PutWindowFloat(typeWindow *window, float tmp) {
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                CurrFontType.Height + 10, 5, 1, window->color_frame, window->color_inside);
        window->paintFlag = 1;
    }
    lcd_active_window(window->x_psition, window->y_position, window->x_psition + window->width,
            window->y_position + CurrFontType.Height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    LCD_PutFloat(window->x_psition, window->y_position, window->color_text, tmp);
}

void LCD_PutWindowWithTitleInt(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, unsigned int tmp) {
    LCD_SetCurrentFont(fTFont);
    unsigned int height = CurrFontType.Height;
    LCD_SetCurrentFont(fTFontforTitle);
    height += CurrFontType.Height + 5; // 5 for space betwen title and tmp value
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                height + 10, 5, 1, window->color_frame, window->color_inside);
        LCD_PutString(window->x_psition, window->y_position, window->color_text, Title);
        window->paintFlag = 1;
    }
    height = CurrFontType.Height + 5;
    LCD_SetCurrentFont(fTFont);
    lcd_active_window(window->x_psition, window->y_position + height, window->x_psition + window->width,
            window->y_position + CurrFontType.Height + height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    LCD_PutInt(window->x_psition, window->y_position + height, window->color_text, tmp);
}

void LCD_PutWindowWithTitleFloat(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, float tmp) {
    LCD_SetCurrentFont(fTFont);
    unsigned int height = CurrFontType.Height;
    LCD_SetCurrentFont(fTFontforTitle);
    height += CurrFontType.Height + 5; // 5 for space betwen title and tmp value
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                height + 10, 5, 1, window->color_frame, window->color_inside);
        LCD_PutString(window->x_psition, window->y_position, window->color_text, Title);
        window->paintFlag = 1;
    }
    height = CurrFontType.Height + 5;
    LCD_SetCurrentFont(fTFont);
    lcd_active_window(window->x_psition, window->y_position + height, window->x_psition + window->width,
            window->y_position + CurrFontType.Height + height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    LCD_PutFloat(window->x_psition, window->y_position + height, window->color_text, tmp);
}

void LCD_PutTemperatureWindow(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, float tmp) {
    LCD_SetCurrentFont(fTFont);
    unsigned int height = CurrFontType.Height;
    LCD_SetCurrentFont(fTFontforTitle);
    height += CurrFontType.Height + 5; // 5 for space betwen title and tmp value
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                height + 10, 5, 1, window->color_frame, window->color_inside);
        LCD_PutString(window->x_psition, window->y_position, window->color_text, Title);
        window->paintFlag = 1;
    }
    height = CurrFontType.Height + 5;
    LCD_SetCurrentFont(fTFont);
    lcd_active_window(window->x_psition, window->y_position + height, window->x_psition + window->width,
            window->y_position + CurrFontType.Height + height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    LCD_PutFloatWithNrPl(window->x_psition, window->y_position + height, window->color_text, 2, tmp); // 2 places after decimal point
}

void LCD_PutTimeWindow(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, struct_RTC_Time *time) {
    LCD_SetCurrentFont(fTFont);
    unsigned int height = CurrFontType.Height;
    LCD_SetCurrentFont(fTFontforTitle);
    height += CurrFontType.Height + 5; // 5 for space betwen title and tmp value
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                height + 10, 5, 1, window->color_frame, window->color_inside);
        LCD_PutString(window->x_psition, window->y_position, window->color_text, Title);
        window->paintFlag = 1;
    }
    height = CurrFontType.Height + 5;
    LCD_SetCurrentFont(fTFont);
    lcd_active_window(window->x_psition, window->y_position + height, window->x_psition + window->width,
            window->y_position + CurrFontType.Height + height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    //    char string[10], tmpstring[4];
    char string[9];
    string[i++] = (time->hour / 10) + '0';
    string[i++] = (time->hour % 10) + '0';
    string[i++] = ':';
    string[i++] = (time->minute / 10) + '0';
    string[i++] = (time->minute % 10) + '0';
    string[i++] = ':';
    string[i++] = (time->second / 10) + '0';
    string[i++] = (time->second % 10) + '0';
    string[i] = 0;
    LCD_PutString(window->x_psition, window->y_position + height, window->color_text, string);
}

void LCD_DrawBMP(int x, int y, char * name) {
    FSFILE *plikBMP;
    unsigned long offset = 0, sizebmp = 0;
    unsigned long width = 0, height = 0, temp;
    unsigned int i;
    unsigned char tmp2 = 0, i2;
    char * wskaznik;
    wskaznik = (char *) &filebuffer;
    plikBMP = FSfopen(name, "r");
    FSfseek(plikBMP, 2, 0);
    FSfread((char*) &sizebmp, 4, 1, plikBMP);
    //    LCD_PutInt(30,50,0,sizebmp);
    FSfseek(plikBMP, 4, 1);
    FSfread((char*) &offset, 4, 1, plikBMP);
    //    LCD_PutInt(30,65,0,offset);
    FSfseek(plikBMP, 4, 1);
    FSfread((char*) &width, 4, 1, plikBMP);
    //    LCD_PutInt(30,95,0,width);
    FSfread((char*) &height, 4, 1, plikBMP);
    //    LCD_PutInt(30,110,0,height);
    FSfseek(plikBMP, offset, 0);
    temp = width * height * 3;
    tmp2 = temp / BuffForBMP;
    lcd_active_window(x, y, x + width - 1, y + height - 1);
    LCD_WriteCommand(MADCTR);
    LCD_WriteData(0b00101000);
    LCD_WriteCommand(RAMWR);
    for (i2 = 0; i2 < tmp2; ++i2) {
        FSfread(wskaznik, BuffForBMP, 1, plikBMP);
        for (i = 0; i < BuffForBMP; ++i) LCD_WriteData(filebuffer[i]);
        temp -= BuffForBMP;
    }
    if (temp > 0) {
        FSfread(wskaznik, temp, 1, plikBMP);
        for (i = 0; i < temp; ++i) LCD_WriteData(filebuffer[i]);
    }
    LCD_WriteCommand(MADCTR);
    LCD_WriteData(0b01101000);
    FSfclose(plikBMP);
}

void LCD_PutTimeWindowInt(typeWindow *window, const fontType *fTFontforTitle, const fontType *fTFont, char *Title, unsigned int time) {
    LCD_SetCurrentFont(fTFont);
    unsigned int height = CurrFontType.Height;
    LCD_SetCurrentFont(fTFontforTitle);
    height += CurrFontType.Height + 5; // 5 for space betwen title and tmp value
    if (!window->paintFlag) {
        LCD_RoundRect(window->x_psition - 5, window->y_position - 5, window->width + 10,
                height + 10, 5, 1, window->color_frame, window->color_inside);
        LCD_PutString(window->x_psition, window->y_position, window->color_text, Title);
        window->paintFlag = 1;
    }
    height = CurrFontType.Height + 5;
    LCD_SetCurrentFont(fTFont);
    lcd_active_window(window->x_psition, window->y_position + height, window->x_psition + window->width,
            window->y_position + CurrFontType.Height + height);
    unsigned int i = (CurrFontType.Height + 1) * (window->width + 1);
    unsigned long color = window->color_inside;
    LCD_WriteCommand(RAMWR);
    while (i) {
        LCD_WriteData((unsigned char) color);
        LCD_WriteData((unsigned char) (color >> 8));
        LCD_WriteData((unsigned char) (color >> 16));
        --i;
    }
    unsigned char hour, minute, second;
    second = time % 60;
    minute = (time / 60) % 60;
    hour = (time / 3600);
    char string[9];
    string[i++] = (hour / 10) + '0';
    string[i++] = (hour % 10) + '0';
    string[i++] = ':';
    string[i++] = (minute / 10) + '0';
    string[i++] = (minute % 10) + '0';
    string[i++] = ':';
    string[i++] = (second / 10) + '0';
    string[i++] = (second % 10) + '0';
    string[i] = 0;
    LCD_PutString(window->x_psition, window->y_position + height, window->color_text, string);
}