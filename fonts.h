/* 
 * File:   fonts.h
 * Author: Krzysiek
 *
 * Created on 29 kwiecie? 2015, 22:40
 */

#ifndef FONTS_H
#define	FONTS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

    typedef struct {
        const uint8_t Width; //Width of the character (px)
        const uint16_t Offset; //Offset of the character's bitmap in the fontType data array

    } charType;

    //Below structure describes generated font

    typedef struct {
        uint8_t Height; //Font height (px)
        uint8_t FirstCharCode; //The first character in the fontType data array
        uint8_t Interspace; //Interspace width (px)
        uint8_t SpaceWidth; //Space character width (px)
        const charType *CharInfo; //Pointer to the array describing subsequent characters
        const uint8_t *Bitmap; //Pointer to the array describing font visual representation

    } fontType;

    //User font data
    extern const charType fontTimesNRcharacters[];
    extern const fontType fontTimesNRfont;
    extern const uint8_t fontTimesNRbitmaps[];
    extern const charType TimesNewRoman24characters[];
    extern const fontType TimesNewRoman24font;
    extern const uint8_t TimesNewRoman24bitmaps[];
    extern const charType Arial10characters[];
    extern const fontType Arial10font;
    extern const uint8_t Arial10bitmaps[];

#ifdef	__cplusplus
}
#endif

#endif	/* FONTS_H */