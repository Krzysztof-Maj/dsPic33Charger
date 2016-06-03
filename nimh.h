/* 
 * File:   nimh.h
 * Author: Krzysiek
 *
 * Created on 16 kwiecie? 2016, 10:37
 */

#ifndef NIMH_H
#define	NIMH_H

#include "hardware.h"


#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct {
        unsigned int current[60];
        unsigned int voltage[60];
        int temperature[60];
        struct_RTC_Time time;
        unsigned char i;
        unsigned char flag;
    } struct_BatteryFile;

    typedef struct {
        unsigned int charge_voltage;
        unsigned long voltage;
        unsigned int current;
        unsigned int peak_voltage;
        unsigned char peak_delay;
        unsigned int dv_ndv;
        unsigned int dv_zdv;
        int temperature;
        unsigned int timer;
        unsigned int user_capacity;
        unsigned int user_current_fast;
        unsigned int user_current_precharge;
        unsigned int time_charge;
        unsigned int constant_for_voltage;
        float electric_charge;
        unsigned char current_chanel;
        unsigned char voltage_chanel;
        unsigned char charger_state;
        unsigned char falut;
        unsigned int battery_pin;
        volatile unsigned int *latch;
    } BATTERY_STRUCT;

    enum CHARGER_STATE {
        CHARGER_IDLE = 0X00, CHARGER_PRECHARGE = 0X01, CHARGER_FAST = 0X02, CHARGER_FINISH = 0X04, CHARGER_TOP_OFF = 0X08,
        CHARGER_MAINTANCE = 0X10, CHARGER_DONE = 0X20, CHARGER_FALUT = 0X40
    };

    enum FALUT_STATE {
        FALUT_NONE, FALUT_NOT_INSERT, FALUT_BATTERY_CHARGED, FALUT_ALKALINE, FALUT_OVERVOLTAGE,
        FALUT_TIMEOUT, FALUT_TEMP_LOW, FALUT_TEMP_HIGH, FALUT_CURRENT_TO_HIGH, FALUT_VOLTAGE_TO_HIGH
    };

#define Battery_MCP 1
#define Battery_ST  2


#define ADC_SAMPLE  16
#define ADC_VREF    3000
#define ADC_BITS    16
#define ADC_MCP_AMP 100      // constant for amplification current MCP
#define ADC_ST_AMP  10      // and for ST

#define dV_ALGORYTHM    5   // mV
#define dT_ALGORYTHM    1   // deg C
#define dV_SAMPLES_NDV  10  // 10 sec
#define dV_SAMPLES_ZDV  600 // 10 min
#define dV_PEAK_DELAY   5   // 5th times

#define BATTERY_NOT_INSERT  400
#define BATTERY_START_FAST  1000
#define BATTERY_NO_CHARGE   1400
#define BATTERY_FINISH_START    1470
#define BATTERY_ALKALINE    1600
#define BATTERY_STOP_CHARGE 1700

#define BATTERY_MIN_TEMP    10
#define BATTERY_MAX_TEMP    50

#define IMPEDANCE_mOHM          200
#define IMPEDANCE_TIME          2
#define IMPEDANCE_MIN_VOLTAGE   1200

#define TIME_PRECHARGE      10  // max 10 min
#define TIME_FAST_CHARGE    60  // max 60 min
#define TIME_FINISH         30  // max 30 min
#define TIME_TOPOFF         30  // max 30 min
#define TIME_MAINTANCE      600 // max 10 hour


    unsigned int GetAdc(unsigned char channel);
    void Battery(void);
    void Battery_DoCharge(void);
    void Battery_SetCurrentBattery(BATTERY_STRUCT *tbs);

    void Measure_Current(void);
    void Measure_Voltage(void);

    void Battery_Init(void);
    void Battery_SaveChargeInFile(struct_BatteryFile *batteryfile, char* filename);
    void Battery_StructForFileActualize(void);

#ifdef	__cplusplus
}
#endif

#endif	/* NIMH_H */

