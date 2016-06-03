#include "hardware.h"

extern BATTERY_STRUCT MCP_struct, ST_struct;
extern struct_FLAGS flags;
extern struct_RTC_Time RTC_Time;
extern struct_RTC_Date RTC_Date;
unsigned int ADCkey;
volatile unsigned int uiTimer1, uiTimer2, uiTimer3;
volatile unsigned char uchTimerForTime, uchTimerBattery, uchTimerForActFile, uchTimerForBatteryFilestruct,uchTimerFalut,uchTimerForBMP;
unsigned int currValST, currValMCP;

typeWindow tWswST = { 10,80,55, 0x00fFfF00, 0x0000ffff, 0, 0};
typeWindow tWswMCP = { 10,130,55, 0x00fFfF00, 0x0000ffff, 0, 0};
typeWindow tWkeyboard = {10,10,85,0x00f0f0f0,0xffFF00,0,0};
typeWindow tWVoltageST = {115,90,80,0,0x00ffbf00,0,0};
typeWindow tWCurrentST = {115,150,80,0,0x009090ff,0,0};
typeWindow tWVoltageMCP = {225,90,80,0,0x00ffbf00,0,0};
typeWindow tWCurrentMCP = {225,150,80,0,0x009090ff,0,0};
typeWindow tWTemperature = {225,10,80,0,0x000000ff,0,0};
typeWindow tWTime = {115,10,80,0,0x0000ff00,0,0};

typeWindow tW_MCP_Voltage = {10,10,80,0,0x91B0C3,0,0};  // colour khaki
typeWindow tW_MCP_Current = {10,70,80,0,0xC0C0C0,0,0};  // silver
typeWindow tW_MCP_ElectricCharge = {10,130,80,0,0x91B0C3,0,0};
typeWindow tW_MCP_Time = {10,190,100,0,0xC0C0C0,0,0};
typeWindow tW_MCP_Capacity = {10,70,80,0,0xC0C0C0,0,0};
typeWindow tW_MCP_Precharge = {10,130,80,0,0x91B0C3,0,0};
typeWindow tW_MCP_Fastcharge = {10,190,80,0,0xC0C0C0,0,0};

typeWindow tW_ST_Voltage = {230,10,80,0,0x91B0C3,0,0};  // colour khaki
typeWindow tW_ST_Current = {230,70,80,0,0xC0C0C0,0,0};  // silver
typeWindow tW_ST_ElectricCharge = {230,130,80,0,0x91B0C3,0,0};
typeWindow tW_ST_Time = {210,190,100,0,0xC0C0C0,0,0};
typeWindow tW_ST_Capacity = {230,70,80,0,0xC0C0C0,0,0};
typeWindow tW_ST_Precharge = {230,130,80,0,0x91B0C3,0,0};
typeWindow tW_ST_Fastcharge = {230,190,80,0,0xC0C0C0,0,0};

typeWindow TWstatusMCP = {110,10,30,0,0xFFFFFF,0,0};
typeWindow TWstatusST = {180,10,30,0,0xFFFFFF,0,0};
typeWindow twMCP_peakVoltage = {110,50,80,0,0xffffff,0,0};
typeWindow twST_peakVoltage = {110,100,80,0,0xffffff,0,0};


void Keyfunction (unsigned char *keystat, unsigned char key, unsigned int waitTime, unsigned int waitRep,
        void (*pressfunction)(void), void (*repfunction)(void));
void Exit_go_setup(void);
void Exit_close_setup (void);
void DownPressFunction (void);
void RightPressFunction (void);
void LeftPressFunction (void);
void UpPressFunction(void);

void (*Exit_Press_short) (void);
void (*Exit_Press_long) (void);
void (*Down_Press_short) (void);
void (*Down_Press_long) (void);
void (*Right_Press_short) (void);
void (*Right_Press_long) (void);
void (*OK_Press_short) (void);
void (*OK_Press_long) (void);
void (*Left_Press_short) (void);
void (*Left_Press_long) (void);
void (*Up_Press_short) (void);
void (*Up_Press_long) (void);

unsigned char uckeyUp, uckeyLeft, uckeyOK, uckeyRight, uckeyDown, uckeyExit;
unsigned int i;
enum emKeyPress KeyPress;
unsigned char dt = 4;

void Hardware_INIT(void) {
    /* Clock configuration */
    CLKDIVbits.PLLPRE = 0; // 8 MHz /2 = 4 MHZ
    PLLFBDbits.PLLDIV = 38; // 4 MHz * 40 = 160 MHz
    CLKDIVbits.PLLPOST = 0; // 160 MHz / 2 = 80 MHz
    RCONbits.SWDTEN = 0; // Wachdog off

    /* ADC configuration*/
    AD1PCFGL = ~(0x060F); // All lines as digital without AN0, AN1, AN2, AN3, AN9, AN10
    AD1CON2bits.VCFG = 1; // Configuration voltage referense, choose external Vref+ and Vss
    AD1CON1bits.AD12B = 1; // 12-bits mode
    AD1CON3bits.ADCS = 4; // Tad = 5*(1/40M)=5*25n=125 ns, min for 12 bit == 117,6 ns
    AD1CON2bits.CHPS = 0; // only CH0 is used (always in 12-bits mode)
    AD1CON1bits.SSRC = 7; // internal clock
    AD1CON3bits.SAMC = 31; // 3 TAD for sampling min
    AD1CON1bits.FORM = 0; // result as integer
    AD1CON1bits.ASAM = 0; // start when SAM bit is set
    AD1CON1bits.ADON = 1; // ADC on

    /* Interrupt */
    INTCON2bits.INT0EP = 1; // falling edge
    IEC0bits.INT0IE = 1; // enable interrupt

    /* INT1 Configure RP4/RB4*/
    TRISBbits.TRISB4 = 1; // INT1 as input
    RPINR0bits.INT1R = 4; // RP4
    INTCON2bits.INT1EP = 1; // falling edge
    IEC1bits.INT1IE = 1; // enable interrupt

    T1CONbits.TCKPS = 1; // prescaler 1/8
    T1CONbits.TCS = 0; // internal source
    T1CONbits.TGATE = 0; // gated time accumulation mode off
    PR1 = 50000; // 40 000 cycles
    IEC0bits.T1IE = 1; // enable interrupt
    IFS0bits.T1IF = 0; // cleared interrupt flags
    T1CONbits.TON = 1; // turn on timer
    //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //

    /////////////////////////////////////////////////////
    //output lines
    //1. RP24 -> SDO1 
    RPOR12bits.RP24R = 7;
    //2. RP12 -> SCK1OUT
    RPOR6bits.RP12R = 8;
    //3. RP25 -> SS1OUT
    RPOR12bits.RP25R = 9;
    //input line
    //4. RP13 <- SDI1
    TRISBbits.TRISB13 = 1;
    RPINR20bits.SDI1R = 13;

    /* Configuration display pins and his initialize */
    TRISC = 0; // data lines D0-D7 --> RC0-RC7
    TRISA &= ~(0x0790); // controls lines
    LCD_Initialize();
    LCD_SetCurrentFont(&fontTimesNRfont);


    LATB |= 0x000C;
    TRISB &= ~(0x000C);
    /* I2C */
    i2c_INIT(100000);
    i2c_MCP4451_INIT();
    i2c_W1(1);
    i2c_W0(1);
    i2c_MCP9804_init(MCP9804_G_adr, MCP9804_res);
    i2c_MCP79412_init();
    i2c_MCP79412_readTime();
    Exit_Press_long = Exit_go_setup;
    i2c_MCP79412_readDate();
}

void Exit_go_setup(void) {
    flags.Setup = 1;
    Exit_Press_short = Exit_close_setup;
    Exit_Press_long = Exit_close_setup;
    Down_Press_short = DownPressFunction;
    Down_Press_long = DownPressFunction;
    Right_Press_short = RightPressFunction;
    Right_Press_long = RightPressFunction;
    OK_Press_short = 0;
    OK_Press_long = 0;
    Left_Press_short = LeftPressFunction;
    Left_Press_long = LeftPressFunction;
    Up_Press_short = UpPressFunction;
    Up_Press_long = UpPressFunction;
    LCD_PaintColor(0xff00ff);
    LCD_RoundRect(105, 60, 100, 145, 10, 1, 0, 0x0000ff00); // st
    LCD_RoundRect(215, 60, 100, 145, 10, 1, 0, 0x0000ff00); // MCP
    LCD_RoundRect(105, 60, 100, 145, 10, 1, 0, 0x0000ff00); // st
    LCD_RoundRect(215, 60, 100, 145, 10, 1, 0, 0x0000ff00); // MCP
    tWCurrentMCP.paintFlag = 0;
    tWCurrentST.paintFlag = 0;
    tWVoltageMCP.paintFlag = 0;
    tWVoltageST.paintFlag = 0;
    tWkeyboard.paintFlag = 0;
    tWswMCP.paintFlag = 0;
    tWswST.paintFlag = 0;
    tWTime.paintFlag = 0;
    tWTemperature.paintFlag = 0;
}

void Exit_close_setup(void) {
    flags.Setup = 0;
    Exit_Press_short = 0;
    Exit_Press_long = Exit_go_setup;
    Down_Press_short = 0;
    Down_Press_long = 0;
    Right_Press_short = 0;
    Right_Press_long = 0;
    OK_Press_short = 0;
    OK_Press_long = 0;
    Left_Press_short = 0;
    Left_Press_long = 0;
    Up_Press_short = 0;
    Up_Press_long = 0;
    LCD_DrawBMP(0, 0, "START.BMP");
    twST_peakVoltage.paintFlag = 0;
    twMCP_peakVoltage.paintFlag = 0;
    TWstatusMCP.paintFlag = 0;
    TWstatusST.paintFlag = 0;
    tW_MCP_Capacity.paintFlag = 0;
    tW_MCP_Current.paintFlag = 0;
    tW_MCP_ElectricCharge.paintFlag = 0;
    tW_MCP_Fastcharge.paintFlag = 0;
    tW_MCP_Precharge.paintFlag = 0;
    tW_MCP_Time.paintFlag = 0;
    tW_MCP_Voltage.paintFlag = 0;
    tW_ST_Capacity.paintFlag = 0;
    tW_ST_Current.paintFlag = 0;
    tW_ST_ElectricCharge.paintFlag = 0;
    tW_ST_Fastcharge.paintFlag = 0;
    tW_ST_Precharge.paintFlag = 0;
    tW_ST_Time.paintFlag = 0;
    tW_ST_Voltage.paintFlag = 0;
}

void OK_insert_screen_MCP(void) {
    flags.MCP_insert = 0;
    flags.MCP_charge = 1;
    Exit_Press_short = 0;
    Exit_Press_long = Exit_go_setup;
    Down_Press_short = 0;
    Down_Press_long = 0;
    Right_Press_short = 0;
    Right_Press_long = 0;
    OK_Press_short = 0;
    OK_Press_long = 0;
    Left_Press_short = 0;
    Left_Press_long = 0;
    Up_Press_short = 0;
    Up_Press_long = 0;
}

void OK_insert_screen_ST(void) {
    flags.ST_insert = 0;
    flags.ST_charge = 1;
    Exit_Press_short = 0;
    Exit_Press_long = Exit_go_setup;
    Down_Press_short = 0;
    Down_Press_long = 0;
    Right_Press_short = 0;
    Right_Press_long = 0;
    OK_Press_short = 0;
    OK_Press_long = 0;
    Left_Press_short = 0;
    Left_Press_long = 0;
    Up_Press_short = 0;
    Up_Press_long = 0;

}

void UpPressFunction(void) {
    if (PORTBbits.RB3) {
        LATBbits.LATB3 = 0;
        return;
    }
    ++currValST;
    i2c_W1(currValST); // st1s10
    LCD_PutWindowWithTitleInt(&tWswST, &Arial10font, &fontTimesNRfont, "ST P1W", currValST);
}

void DownPressFunction(void) {
    if (!currValST) return;
    --currValST;
    i2c_W1(currValST); // st1s10
    LCD_PutWindowWithTitleInt(&tWswST, &Arial10font, &fontTimesNRfont, "ST P1W", currValST);
}

void RightPressFunction(void) {
    if (PORTBbits.RB2) {
        LATBbits.LATB2 = 0;
        return;
    }
    ++currValMCP;
    i2c_W0(currValMCP); // MCP
    LCD_PutWindowWithTitleInt(&tWswMCP, &Arial10font, &fontTimesNRfont, "MCP P0W", currValMCP);
}

void LeftPressFunction(void) {
    if (!currValMCP) return;
    --currValMCP;
    i2c_W0(currValMCP); // MCP
    LCD_PutWindowWithTitleInt(&tWswMCP, &Arial10font, &fontTimesNRfont, "MCP P0W", currValMCP);
}

void Keyfunction(unsigned char *keystat, unsigned char key, unsigned int waitTime, unsigned int waitRep,
        void (*pressfunction) (void), void (*repfunction)(void)) {

    enum {
        idle, press, go, go_rep, rep
    };
    if (!*keystat && key) {
        *keystat = press;
        uiTimer1 = 2;
    }
    if (*keystat) {
        if (!uiTimer1 && *keystat == press && key) {
            *keystat = go;
            uiTimer1 = 5;
        } else if (*keystat < rep && !key && *keystat > press) {
            if (pressfunction) pressfunction();
            *keystat = idle;
        } else if (!uiTimer1 && *keystat == go && key) {
            *keystat = go_rep;
            uiTimer1 = waitTime;
        } else if (!uiTimer1 && *keystat > go && key) {
            uiTimer1 = waitRep;
            if (repfunction) repfunction();
            *keystat = rep;
        }
    }
    if (*keystat && !key) *keystat = idle;
}

void KeyCheck(void) {
    ADCkey = GetAdc(3) >> 4;
    if (ADCkey > KeyExit) KeyPress = eanykey;
    else if (ADCkey > KeyDown) KeyPress = ekeyExit;
    else if (ADCkey > KeyRight) KeyPress = ekeyDown;
    else if (ADCkey > KeyOK) KeyPress = ekeyRight;
    else if (ADCkey > KeyLeft) KeyPress = ekeyOK;
    else if (ADCkey > KeyUp) KeyPress = ekeyLeft;
    else KeyPress = ekeyUp;
    Keyfunction(&uckeyExit, KeyPress & ekeyExit, 100, 1000, Exit_Press_short, Exit_Press_long);
    Keyfunction(&uckeyDown, KeyPress & ekeyDown, 100, 10, Down_Press_short, Down_Press_long);
    Keyfunction(&uckeyRight, KeyPress & ekeyRight, 100, 10, Right_Press_short, Right_Press_long);
    Keyfunction(&uckeyOK, KeyPress & ekeyOK, 100, 10, OK_Press_short, OK_Press_long);
    Keyfunction(&uckeyLeft, KeyPress & ekeyLeft, 100, 10, Left_Press_short, Left_Press_long);
    Keyfunction(&uckeyUp, KeyPress & ekeyUp, 100, 10, Up_Press_short, Up_Press_long);
}

void Measure_ADC(void) {
    Battery_SetCurrentBattery(&ST_struct);
    Measure_Current();
    Measure_Voltage();
    Battery_SetCurrentBattery(&MCP_struct);
    Measure_Current();
    Measure_Voltage();
}

void LCD_Draw_layer(void) {
    if (flags.BMP_Hand) if (!uchTimerForBMP) {
            flags.BMP_Hand = 0;
            LCD_DrawBMP(0, 0, "HAND.BMP");
        }
    if (flags.Setup) {
        LCD_PutWindowWithTitleInt(&tWkeyboard, &Arial10font, &fontTimesNRfont, "Napiecie klawiszy", ADCkey);
        LCD_PutWindowWithTitleInt(&tWVoltageST, &Arial10font, &fontTimesNRfont, "Napiecie ST", (ST_struct.charge_voltage >> 4));
        LCD_PutWindowWithTitleInt(&tWCurrentST, &Arial10font, &fontTimesNRfont, "Prad ST", (unsigned int) ST_struct.current / 21.8 + 7);
        LCD_PutWindowWithTitleInt(&tWVoltageMCP, &Arial10font, &fontTimesNRfont, "Napiecie MCP", (unsigned int) (MCP_struct.charge_voltage >> 4));
        LCD_PutWindowWithTitleInt(&tWCurrentMCP, &Arial10font, &fontTimesNRfont, "Prad MCP", (unsigned int) MCP_struct.current / 24 + 4);
        LCD_PutTemperatureWindow(&tWTemperature, &Arial10font, &Arial10font, "Temperatura", (float) i2c_MCP9804(MCP9804_G_adr) / 16);
    } else {
        if (flags.MCP_insert) {
            if (!OK_Press_short) {
                OK_Press_short = OK_insert_screen_MCP;
            }
            LCD_PutWindowWithTitleInt(&tW_MCP_Voltage, &Arial10font, &TimesNewRoman24font, "Voltage", (unsigned int) (MCP_struct.voltage >> 4) * 3000.0 / 4095.0);
            LCD_PutWindowWithTitleInt(&tW_MCP_Capacity, &Arial10font, &TimesNewRoman24font, "Capacity", MCP_struct.user_capacity);
            LCD_PutWindowWithTitleInt(&tW_MCP_Precharge, &Arial10font, &TimesNewRoman24font, "Precharge", (unsigned int) MCP_struct.user_current_precharge / 24 + 4);
            LCD_PutWindowWithTitleInt(&tW_MCP_Fastcharge, &Arial10font, &TimesNewRoman24font, "Fast charge", (unsigned int) MCP_struct.user_current_fast / 24 + 4);

        }
        if (flags.ST_insert) {
            LCD_PutWindowWithTitleInt(&tW_ST_Voltage, &Arial10font, &TimesNewRoman24font, "Voltage", (unsigned int) (ST_struct.voltage >> 4) * 3000.0 / 4095.0);
            LCD_PutWindowWithTitleInt(&tW_ST_Capacity, &Arial10font, &TimesNewRoman24font, "Capacity", ST_struct.user_capacity);
            LCD_PutWindowWithTitleInt(&tW_ST_Precharge, &Arial10font, &TimesNewRoman24font, "Precharge", (unsigned int) (ST_struct.user_current_precharge / 21.8 + 7));
            LCD_PutWindowWithTitleInt(&tW_ST_Fastcharge, &Arial10font, &TimesNewRoman24font, "Fast charge", (unsigned int) (ST_struct.user_current_fast / 21.8 + 7));
            if (!OK_Press_short) {
                OK_Press_short = OK_insert_screen_ST;
            }
        }
        if (flags.MCP_charge) {
            LCD_PutWindowWithTitleInt(&tW_MCP_Voltage, &Arial10font, &TimesNewRoman24font, "Voltage", (unsigned int) (MCP_struct.voltage >> 4) * 3000.0 / 4096.0);
            LCD_PutWindowWithTitleInt(&tW_MCP_Current, &Arial10font, &TimesNewRoman24font, "Current", (unsigned int) (MCP_struct.current / 24 + 4));
            LCD_PutWindowWithTitleInt(&tW_MCP_ElectricCharge, &Arial10font, &TimesNewRoman24font, "Electric charge", (unsigned int) MCP_struct.electric_charge);
            LCD_PutTimeWindowInt(&tW_MCP_Time, &Arial10font, &TimesNewRoman24font, "Time", MCP_struct.time_charge);
        }
        if (flags.ST_charge) {
            LCD_PutWindowWithTitleInt(&tW_ST_Voltage, &Arial10font, &TimesNewRoman24font, "Voltage", (unsigned int) (ST_struct.voltage >> 4) * 3000.0 / 4096.0);
            LCD_PutWindowWithTitleInt(&tW_ST_Current, &Arial10font, &TimesNewRoman24font, "Current", (unsigned int) (ST_struct.current / 21.8 + 7));
            LCD_PutWindowWithTitleInt(&tW_ST_ElectricCharge, &Arial10font, &TimesNewRoman24font, "Electric charge", (unsigned int) ST_struct.electric_charge);
            LCD_PutTimeWindowInt(&tW_ST_Time, &Arial10font, &TimesNewRoman24font, "Time", ST_struct.time_charge);
        }

        LCD_SetCurrentFont(&fontTimesNRfont);
        LCD_PutWindow(&TWstatusMCP, MCP_struct.charger_state);
        LCD_PutWindow(&TWstatusST, ST_struct.charger_state);
        LCD_PutWindow(&twMCP_peakVoltage, (MCP_struct.peak_voltage >> 4) * 3000.0 / 4096.0);
        LCD_PutWindow(&twST_peakVoltage, (ST_struct.peak_voltage >> 4) * 3000.0 / 4096.0);
    }
    uiTimer3 = 40;
}