/* 
 * File:   hardware.h
 * Author: Krzysiek
 *
 * Created on 25 kwiecie? 2016, 23:31
 */

#ifndef HARDWARE_H
#define	HARDWARE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <libpic30.h>
#include <p33FJ128GP804.h>
#include "LCD_NokiaE51.h"
#include "fonts.h"
#include "i2c.h"
#include "nimh.h"
#include "SDCard/FSIO.h"

#define FCY 40000000UL
typedef unsigned char u08;
typedef unsigned int  u16;
#define KeyUp       1500        // tresholds for key ADC
#define KeyLeft     2600
#define KeyOK       3100
#define KeyRight    3400
#define KeyDown     3580
#define KeyExit     3700

/////////////// FLAGS /////////////////////////

    typedef struct {
        u16 MCP_insert : 1;
        u16 ST_insert : 1;
        u16 MCP_charge : 1;
        u16 ST_charge : 1;
        u16 LCD_refresh : 1;
        u16 BMP_small_MCP : 1;
        u16 BMP_small_ST : 1;
        u16 Setup : 1;
        u16 BMP_Hand : 1;
    } struct_FLAGS;

    enum emKeyPress {
        eanykey, ekeyUp = 1, ekeyLeft = 2, ekeyOK = 4, ekeyRight = 8, ekeyDown = 16, ekeyExit = 32
    };

    void Hardware_INIT(void);
    void KeyCheck(void);

    void Measure_ADC(void);
    void LCD_Draw_layer(void);

#ifdef	__cplusplus
}
#endif

#endif	/* HARDWARE_H */

