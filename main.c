/* 
 * File:   main.c
 * Author: Krzysiek
 *
 * Created on 11 pa?dziernik 2015, 23:33
 */

#include "hardware.h"

// DSPIC33FJ128GP804 Configuration Bit Settings

// 'C' source line config statements
// FBS
#pragma config BWRP = WRPROTECT_ON      // Boot Segment Write Protect (Boot segment is write-protected)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WRPROTECT_OFF     // Secure Segment Program Write Protect (Secure segment may be written)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Mode (Primary Oscillator (XT, HS, EC) w/ PLL)
#pragma config IESO = OFF               // Internal External Switch Over Mode (Start-up device with user-selected oscillator source)

// FOSC
#pragma config POSCMD = XT              // Primary Oscillator Source (HS Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Both Clock Switching and Fail-Safe Clock Monitor are disabled)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)

// FICD
#pragma config ICS = PGD2               // Comm Channel Select (Communicate on PGC2/EMUC2 and PGD2/EMUD2)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)

extern volatile unsigned int uiTimer1, uiTimer2, uiTimer3;
extern volatile unsigned char uchTimerForTime, uchTimerBattery, uchTimerForActFile, uchTimerForBatteryFilestruct,uchTimerFalut,uchTimerForBMP;
extern struct_FLAGS flags;
extern typeWindow tWTime;
extern BATTERY_STRUCT MCP_struct, ST_struct;
extern struct_RTC_Time RTC_Time;
extern struct_RTC_Date RTC_Date;
extern struct_BatteryFile MCP_Batteryfile_1, MCP_Batteryfile_2, ST_Batteryfile_1, ST_Batteryfile_2;

int main(void)
{
    Hardware_INIT();
    Battery_Init();

    while(!FSInit());
    LCD_DrawBMP(0,0,"START.BMP");
    uchTimerForBMP = 3;
    flags.BMP_Hand = 1;
    uchTimerForActFile = 60;
    while (1) {
        if (!uchTimerForTime) {
            i2c_MCP79412_readTime();
            if (flags.Setup) LCD_PutTimeWindow(&tWTime, &Arial10font, &Arial10font, "Zegarek", &RTC_Time);
            if (flags.MCP_charge) {
                ++MCP_struct.time_charge;
            }
            if (flags.ST_charge) {
                ++ST_struct.time_charge;
            }
            if (!MCP_struct.timer) --MCP_struct.timer;
            if (!ST_struct.timer) --ST_struct.timer;
            uchTimerForTime = 1;
        }
        
        KeyCheck();
        Measure_ADC();

        if (!uchTimerBattery) {
            Battery();
            uchTimerBattery = 1;
        }
        if (!uiTimer3) LCD_Draw_layer();
        if (!uchTimerForBatteryFilestruct) {
            Battery_StructForFileActualize();
            uchTimerForBatteryFilestruct = 1;
        }
        if (!uchTimerForActFile) {
            if (MCP_Batteryfile_1.i == 60) Battery_SaveChargeInFile((struct_BatteryFile*) & MCP_Batteryfile_1, "MCP_HISTORY_CHARGER.TXT");
            if (MCP_Batteryfile_2.i == 60) Battery_SaveChargeInFile((struct_BatteryFile*) & MCP_Batteryfile_2, "MCP_HISTORY_CHARGER.TXT");
            if (ST_Batteryfile_1.i == 60) Battery_SaveChargeInFile((struct_BatteryFile*) & ST_Batteryfile_1, "ST_HISTORY_CHARGER.TXT");
            if (ST_Batteryfile_2.i == 60) Battery_SaveChargeInFile((struct_BatteryFile*) & ST_Batteryfile_2, "ST_HISTORY_CHARGER.TXT");
            uchTimerForActFile = 60;
        }
    }
}

void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void) {
    LATBbits.LATB2 = 1; // turn off MCP i ST
    LATBbits.LATB3 = 1;
    IFS0bits.INT0IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _INT1Interrupt(void) { // RTC interrupt
    if (uchTimerForTime) --uchTimerForTime;
    if (uchTimerForActFile) --uchTimerForActFile;
    if (uchTimerForBatteryFilestruct) --uchTimerForBatteryFilestruct;
    if (uchTimerBattery) --uchTimerBattery;
    if (!uchTimerFalut) --uchTimerFalut;
    if (!uchTimerForBMP) --uchTimerForBMP;
    IFS1bits.INT1IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    if (uiTimer1) --uiTimer1; // Timer for key function
    if (uiTimer2) --uiTimer2; // Timer for window
    if (uiTimer3) --uiTimer3; // Timer for graphic
    IFS0bits.T1IF = 0;
}