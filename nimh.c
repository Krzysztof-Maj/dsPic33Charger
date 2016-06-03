#include "nimh.h"

BATTERY_STRUCT *bs;
BATTERY_STRUCT MCP_struct, ST_struct;
extern struct_RTC_Time RTC_Time;
extern struct_RTC_Date RTC_Date;
const struct_BatteryFile MCP_Batteryfile_1, MCP_Batteryfile_2, ST_Batteryfile_1, ST_Batteryfile_2;
extern volatile unsigned char uchTimerForTime, uchTimerBattery, uchTimerForActFile, uchTimerForBatteryFilestruct,uchTimerFalut;
struct_BatteryFile *batteryfile;
struct_FLAGS flags;

extern unsigned char dt;
unsigned int read;

const unsigned int const_battery_not_insert =   (BATTERY_NOT_INSERT * 65536) / ADC_VREF;
const unsigned int const_battery_start_fast =   (BATTERY_START_FAST * 65536) / ADC_VREF;
const unsigned int const_battery_no_charge  =   (BATTERY_NO_CHARGE * 65536) / ADC_VREF;
const unsigned int const_battery_alkaline   =   (BATTERY_ALKALINE *65536) / ADC_VREF;
const unsigned int const_battery_stop_charge =  (BATTERY_STOP_CHARGE *65536) / ADC_VREF;
const unsigned int const_battery_finish_start = (BATTERY_FINISH_START * 65536) / ADC_VREF;

const unsigned int const_impedance_min_voltage = (IMPEDANCE_MIN_VOLTAGE * 65536) / ADC_VREF;
const unsigned char const_dv_algorythm = (dV_ALGORYTHM * 65536) / ADC_VREF;

const unsigned int const_time_precharge =   TIME_PRECHARGE * 60;
const unsigned int const_time_fast_charge = TIME_FAST_CHARGE * 60;
const unsigned int const_time_finish =      TIME_FINISH * 60;
const unsigned int const_time_topoff =      TIME_TOPOFF * 60;
const unsigned int const_time_maintance =   (unsigned int) TIME_MAINTANCE * 60;

void Measure_Temperature(void);
void Battery_SetCurrent (unsigned int current);
void Battery_FalutCheck (void);
void Battery_CurrentOFF (void);                 
void Battery_CurrentON (void);                  
void Battery_Detection (void);
void Battery_State (void);
void Battery_ErrorCheck (void);
void Battery_Precharge(void);
void Battery_FastCharge(void);
void Battery_FinishCharge (void);
void Battery_TopOff (void);
void Battery_Maintance (void);

unsigned int GetAdc(unsigned char channel) {
    AD1CHS0bits.CH0SA = channel; // ANi --> channel CH0 pos+
    asm("NOP");
    AD1CON1bits.SAMP = 1; // start samples
    while (!AD1CON1bits.DONE);
    unsigned int ADCtmp = 0;
    unsigned char i;
    for (i = 0; i < 16; ++i) {
        AD1CON1bits.SAMP = 1; // start samples
        while (!AD1CON1bits.DONE);
        ADCtmp += ADC1BUF0;
    }
    return ADCtmp;
}

void Battery_SetCurrentBattery(BATTERY_STRUCT* tbs) {
    bs = tbs;
}

void Battery_DoCharge(void) {
    Measure_Current();
    Measure_Voltage();
    Measure_Temperature();
    Battery_Detection();
    Battery_State();
}

void Battery_CurrentOFF(void) {
    *(bs->latch) |= bs->battery_pin; // turn off dc-dc converter
}

void Battery_CurrentON(void) {
    *(bs->latch) &= ~(bs->battery_pin); // turn on converter
    Measure_Current();
    if (bs->current > 65440) {
        Battery_CurrentOFF();
        bs->charger_state = 17;
        //        bs->charger_state = CHARGER_FALUT;
        bs->falut = FALUT_CURRENT_TO_HIGH;
        return;
    }
    Measure_Voltage();
    if (bs->voltage > 65440) {
        Battery_CurrentOFF();
        bs->charger_state = 18;
        //        bs->charger_state = CHARGER_FALUT;
        bs->falut = FALUT_VOLTAGE_TO_HIGH;
    }
}

void Measure_Current(void) {
    read = GetAdc(bs->current_chanel);
    if (read > 65440) bs->current = read;
    if (bs->current > read) bs->current -= (bs->current - read) >> dt;
    else bs->current += (read - bs->current) >> dt;
}

void Measure_Voltage(void) {
    read = GetAdc(bs->voltage_chanel);
    if (read > 65400) {
        bs->voltage = read;
        bs->charge_voltage = read;
        bs->falut = FALUT_VOLTAGE_TO_HIGH;
        //        bs->charger_state = CHARGER_FALUT;
        bs->charger_state = 19;
        uchTimerFalut = 10;
        return;
    }
    if (bs->charge_voltage > read) bs->charge_voltage -= (bs->charge_voltage - read) >> dt;
    else bs->charge_voltage += (read - bs->charge_voltage) >> dt;
    bs->voltage = bs->charge_voltage - (bs->current / bs->constant_for_voltage);
}

void Measure_Temperature(void) {
    bs->temperature = i2c_MCP9804(MCP9804_G_adr);
}

void Battery_Detection(void) {
    if (bs->charger_state == CHARGER_IDLE) {
        // If voltage is lowest than 0,4 V don't charge
        if (bs->voltage < const_battery_not_insert) {
            bs->charger_state = CHARGER_IDLE;
            bs->falut = FALUT_NOT_INSERT;
        } else {
            // If voltage is lowest than 1,4V go to precharge
            if (bs->voltage < const_battery_no_charge) {
                bs->falut = FALUT_NONE;
                bs->charger_state = CHARGER_PRECHARGE;
                bs->timer = const_time_precharge;
                Battery_SetCurrent(bs->user_current_precharge);
            } else if (bs->voltage < const_battery_alkaline) {
                bs->charger_state = CHARGER_DONE;
                bs->falut = FALUT_BATTERY_CHARGED;
            } else {
                bs->charger_state = CHARGER_IDLE;
                bs->falut = FALUT_ALKALINE;
            }

            if ((bs->temperature >> 4) < BATTERY_MIN_TEMP || (bs->temperature >> 4) > BATTERY_MAX_TEMP) {
                //                bs->charger_state = CHARGER_FALUT;
                bs->charger_state = 20;
                if (bs->temperature < BATTERY_MIN_TEMP) bs->falut = FALUT_TEMP_LOW;
                else bs->falut = FALUT_TEMP_HIGH;
            }
        }
    } else if (bs->voltage < const_battery_not_insert) {
        bs->charger_state = CHARGER_IDLE;
        bs->falut = FALUT_NOT_INSERT;
    }
}

void Battery_State(void) {
    switch (bs->charger_state) {
        case CHARGER_PRECHARGE: Battery_Precharge();
            break;
        case CHARGER_FAST: Battery_FastCharge();
            break;
        case CHARGER_FINISH: Battery_FinishCharge();
            break;
        case CHARGER_TOP_OFF: Battery_TopOff();
            break;
        case CHARGER_MAINTANCE: Battery_Maintance();
            break;
    }
    Battery_FalutCheck();
}

void Battery_FalutCheck(void) {
    //    if (bs->voltage < bs->voltage) bs->voltage = bs->voltage;

    //    if (bs->time_charge >= IMPEDANCE_TIME)
    //    {
    //        if (bs->voltage > const_battery_stop_charge)
    //        {
    //            bs->charger_state = CHARGER_FALUT;
    //            bs->falut = FALUT_OVERVOLTAGE;
    //        }
    //
    ////        if (bs->charge_voltage > bs->voltage && bs->charge_voltage > const_impedance_min_voltage)
    ////            if (bs->charge_voltage - bs->voltage > )
    //    }
}

void Battery_Precharge(void) {
    if (!bs->timer) {
        //        bs->charger_state = CHARGER_FALUT;
        bs->charger_state = 21;
        bs->falut = FALUT_TIMEOUT;
    }
    if (bs->voltage > const_battery_start_fast) {
        bs->charger_state = CHARGER_FAST;
        bs->timer = const_time_fast_charge;
        //        bs->time_electric = 0;
        Battery_SetCurrent(bs->user_current_fast);
    }
}

void Battery_FastCharge(void) {
    if (!bs->timer) {
        //        bs->charger_state = CHARGER_FALUT;
        bs->charger_state = 22;
        bs->falut = FALUT_TIMEOUT;
    }
    if (bs->voltage > const_battery_finish_start) {
        bs->charger_state = CHARGER_FINISH;
        bs->timer = const_time_finish;
        bs->peak_voltage = bs->voltage;
        bs->dv_ndv = dV_SAMPLES_NDV;
        bs->dv_zdv = dV_SAMPLES_ZDV;
        //        Battery_SetCurrent(bs->user_current_fast);
    }
}

void Battery_FinishCharge(void) {
    unsigned int temp = bs->voltage;
    if (bs->peak_voltage < temp) {
        if (bs->peak_delay) bs->peak_delay--;
        else {
            bs->peak_delay = dV_PEAK_DELAY;
            bs->dv_zdv = dV_SAMPLES_ZDV;
            bs->peak_voltage = temp;
        }
    } else bs->peak_delay = dV_PEAK_DELAY;

    if (bs->dv_zdv) bs->dv_zdv--;
    else {
        bs->charger_state = 17;
        //        bs->charger_state = CHARGER_MAINTANCE;
        bs->timer = const_time_maintance;
        Battery_SetCurrent(bs->user_capacity / 40);
    }

    if (temp < bs->peak_voltage - const_dv_algorythm) {
        if (bs->dv_ndv) bs->dv_ndv--;
        else {
            bs->charger_state = 18;
            //            bs->charger_state = CHARGER_MAINTANCE;
            bs->timer = const_time_maintance;
            Battery_SetCurrent(bs->user_capacity / 40);
        }
    }

    if (!bs->timer) {
        bs->charger_state = 19;
        //        bs->charger_state = CHARGER_MAINTANCE;
        bs->timer = const_time_maintance;
        Battery_SetCurrent(bs->user_capacity / 40);
    }
}

void Battery_Maintance(void) {
    if (!bs->timer) bs->charger_state = CHARGER_DONE;
}

void Battery_TopOff(void) {
    if (!bs->timer) {
        bs->charger_state = CHARGER_MAINTANCE;
        bs->timer = const_time_maintance;
        Battery_SetCurrent(bs->user_capacity / 40);
    }
}

void Battery_SetCurrent(unsigned int current) {
    unsigned char temp = 1;
    unsigned int tempint = bs->current, ii;
    if (bs->battery_pin & (1 << 2)) // MCP conected to RB2, pw0 -- mcp,
    {
        i2c_W0(temp);
        Battery_CurrentON();
        for (ii = 2; ii < (current / 213); ++ii) {
            i2c_W0(ii);
            Measure_Current();
        }
        //        while (current > tempint)
        //        {
        //            i2c_W0(++temp);
        //            tempint = GetAdc(bs->current_chanel);
        Measure_Voltage();
        if (bs->voltage > const_battery_stop_charge) {
            //                bs->charger_state = CHARGER_FALUT;
            bs->charger_state = 23;
            Battery_CurrentOFF();
            bs->falut = FALUT_VOLTAGE_TO_HIGH;
            return;
        }
        //        }
        //        while (current < tempint)
        //        {
        //            if (temp == 1) break;
        //            i2c_W0(--temp);
        //            tempint = GetAdc(bs->current_chanel);
        //        }
        //        bs->current = tempint;

    }
    if (bs->battery_pin & (1 << 3)) // ST connected to RB3, pw1 -- ST
    {
        i2c_W1(temp);
        Battery_CurrentON();
        while (current > tempint) {
            i2c_W1(++temp);
            for (ii = 50; ii; --ii);
            tempint = GetAdc(bs->current_chanel); // stabilise ADC
            Measure_Current();
            Measure_Voltage();
            if (bs->voltage > const_battery_stop_charge) {
                Battery_CurrentOFF();
                //                bs->charger_state = CHARGER_FALUT;
                bs->charger_state = 24;
                bs->falut = FALUT_VOLTAGE_TO_HIGH;
                return;
            }
        }
        while (current < tempint) {
            if (temp == 1) break;
            i2c_W1(--temp);
            for (ii = 1000; ii; --ii)tempint = GetAdc(bs->current_chanel);

        }
        bs->current = tempint;
    }
}

void Battery_Init(void) {
    MCP_struct.battery_pin = (1 << 2); // MCP conneted to rb2;
    MCP_struct.latch = &LATB;
    MCP_struct.charge_voltage = 0;
    MCP_struct.charger_state = CHARGER_IDLE;
    MCP_struct.current = 0;
    MCP_struct.current_chanel = 9; // AN9 for current
    MCP_struct.dv_ndv = dV_SAMPLES_NDV;
    MCP_struct.dv_zdv = dV_SAMPLES_ZDV;
    MCP_struct.falut = FALUT_NONE;
    MCP_struct.peak_delay = dV_PEAK_DELAY;
    MCP_struct.peak_voltage = 0;
    MCP_struct.electric_charge = 0;
    MCP_struct.temperature = 0;
    MCP_struct.time_charge = 0;
    MCP_struct.timer = 0;
    MCP_struct.user_capacity = 2000;
    MCP_struct.user_current_fast = (2000 - 4) * 12; // current c/2
    MCP_struct.user_current_precharge = (unsigned int) (200 - 4) * 24;
    MCP_struct.voltage = 0;
    MCP_struct.voltage_chanel = 2; // AN2
    MCP_struct.constant_for_voltage = ADC_MCP_AMP;

    ST_struct.battery_pin = (1 << 3); // RB3
    ST_struct.latch = &LATB;
    ST_struct.charge_voltage = 0;
    ST_struct.charger_state = CHARGER_IDLE;
    ST_struct.current = 0;
    ST_struct.current_chanel = 10; // AN10
    ST_struct.dv_ndv = dV_SAMPLES_NDV;
    ST_struct.dv_zdv = dV_SAMPLES_ZDV;
    ST_struct.falut = FALUT_NONE;
    ST_struct.peak_delay = dV_PEAK_DELAY;
    ST_struct.peak_voltage = 0;
    ST_struct.electric_charge = 0;
    ST_struct.temperature = 0;
    ST_struct.time_charge = 0;
    ST_struct.timer = 0;
    ST_struct.user_capacity = 2000;
    ST_struct.user_current_fast = ((2000 - 7) * 21.8) / 2; // current c/2
    ST_struct.user_current_precharge = (200 - 7)*21, 8; // 200 mA
    ST_struct.voltage = 0;
    ST_struct.voltage_chanel = 1; // AN1
    ST_struct.constant_for_voltage = ADC_ST_AMP;

}

void Battery_SaveChargeInFile(struct_BatteryFile *batteryfile, char* filename) {
    char text [40], tmp[6], enter[3];
    enter[0] = 0x0D; // CR
    enter[1] = 0x0A; // LF
    enter[2] = 0;
    unsigned char i = 0, i2 = 0, i3;
    batteryfile->i -= 1;
    text[i++] = 2 + '0';
    text[i++] = '0';
    text[i++] = RTC_Date.year / 10 + '0';
    text[i++] = RTC_Date.year % 10 + '0';
    text[i++] = '-';
    text[i++] = RTC_Date.month / 10 + '0';
    text[i++] = RTC_Date.month % 10 + '0';
    text[i++] = '-';
    text[i++] = RTC_Date.day / 10 + '0';
    text[i++] = RTC_Date.day % 10 + '0';
    text[i++] = ';';
    FSFILE *plik;
    plik = FSfopen(filename, "a");
    for (i2 = 0; i2 < batteryfile->i; ++i2) {
        i3 = i;
        text[i3++] = batteryfile->time.hour / 10 + '0';
        text[i3++] = batteryfile->time.hour % 10 + '0';
        text[i3++] = ':';
        text[i3++] = batteryfile->time.minute / 10 + '0';
        text[i3++] = batteryfile->time.minute % 10 + '0';
        text[i3++] = ':';
        text[i3++] = batteryfile->time.second / 10 + '0';
        text[i3++] = batteryfile->time.second % 10 + '0';
        text[i3++] = ';';
        text[i3++] = 0;
        itoa(tmp, batteryfile->temperature[i2], 10);
        strcat(text, tmp);
        strcat(text, ";");
        itoa(tmp, batteryfile->voltage[i2], 10);
        strcat(text, tmp);
        strcat(text, ";");
        itoa(tmp, batteryfile->current[i2], 10);
        strcat(text, tmp);
        strcat(text, ";");
        strcat(text, enter);

        batteryfile->time.second += 1;
        if (batteryfile->time.second == 60) {
            batteryfile->time.second = 0;
            batteryfile->time.minute += 1;
            if (batteryfile->time.minute == 60) {
                batteryfile->time.minute = 0;
                batteryfile->time.hour += 1;
            }
        }
        FSfwrite(text, strlen(text), 1, plik);
    }
    FSfclose(plik);
    batteryfile->i = 0;
}

static void StructForFileActualize(struct_BatteryFile * batteryfile_1, struct_BatteryFile *batteryfile_2, BATTERY_STRUCT *batterystruct) {
    if (batteryfile_1->flag && batteryfile_1->i < 60) {
        if (!batteryfile_1->i) {
            batteryfile_1->time = RTC_Time;
        }
        if (batterystruct->battery_pin == (1 << 2)) {
            batteryfile_1->current[batteryfile_1->i] = batterystruct->current / 24 + 4;
            batteryfile_1->voltage[batteryfile_1->i] = (batterystruct->voltage >> 4) * 3000.0 / 4095.0;
        } else if (batterystruct->battery_pin == (1 << 3)) {
            batteryfile_1->current[batteryfile_1->i] = batterystruct->current / 21.8 + 7;
            batteryfile_1->voltage[batteryfile_1->i] = (batterystruct->voltage >> 4) * 3000.0 / 4095.0;
        }
        batteryfile_1->temperature[batteryfile_1->i] = batterystruct->temperature / 40;
        batteryfile_1->i += 1;
        if (batteryfile_1->i == 60) {
            batteryfile_1->flag = 0;
            batteryfile_2->flag = 1;
        }
    } else if (batteryfile_2->flag && batteryfile_2->i < 60) {
        if (!batteryfile_2->i) {
            batteryfile_2->time = RTC_Time;
        }
        if (batterystruct->battery_pin == (1 << 2)) {
            batteryfile_2->current[batteryfile_2->i] = batterystruct->current / 24 + 4;
            batteryfile_2->voltage[batteryfile_2->i] = (batterystruct->voltage >> 4) * 3000.0 / 4095.0;
            batteryfile_2->temperature[batteryfile_2->i] = i2c_MCP9804(MCP9804_G_adr);
        } else if (batterystruct->battery_pin == (1 << 3)) {
            batteryfile_2->current[batteryfile_2->i] = batterystruct->current / 21.8 + 7;
            batteryfile_2->voltage[batteryfile_2->i] = (batterystruct->voltage >> 4) * 3000.0 / 4095.0;
            batteryfile_2->temperature[batteryfile_2->i] = i2c_MCP9804(MCP9804_G_adr);
        }

        batteryfile_2->i += 1;
        if (batteryfile_2->i == 60) {
            batteryfile_2->flag = 0;
            batteryfile_1->flag = 1;
        }
    }
}

void Battery_StructForFileActualize(void) {
    StructForFileActualize((struct_BatteryFile*) & MCP_Batteryfile_1, (struct_BatteryFile*) & MCP_Batteryfile_2, &MCP_struct);
    StructForFileActualize((struct_BatteryFile*) & ST_Batteryfile_1, (struct_BatteryFile*) & ST_Batteryfile_2, &ST_struct);
}

void Battery(void) {
    if (MCP_struct.falut == FALUT_VOLTAGE_TO_HIGH) {
        if (!uchTimerFalut) {
            MCP_struct.falut = FALUT_NONE;
            MCP_struct.charger_state = CHARGER_IDLE;
            flags.MCP_charge = 0;
            flags.MCP_insert = 0;
        }
    }
    if (ST_struct.falut == FALUT_VOLTAGE_TO_HIGH) {
        if (!uchTimerFalut) {
            ST_struct.falut = FALUT_NONE;
            ST_struct.charger_state = CHARGER_IDLE;
            flags.ST_charge = 0;
            flags.ST_insert = 0;
        }
    }
    if (MCP_struct.voltage > const_battery_not_insert && (!flags.MCP_insert && !flags.MCP_charge) && (MCP_struct.falut == FALUT_NONE)) {
        flags.MCP_insert = 1;
    }
    if (ST_struct.voltage > const_battery_not_insert && (!flags.ST_insert && !flags.ST_charge) && (ST_struct.falut == FALUT_NONE)) {
        flags.ST_insert = 1;
    }
    if (flags.MCP_charge) {
        Battery_SetCurrentBattery(&MCP_struct);
        Battery_DoCharge();
        bs->electric_charge += ((bs->current / 24.0) + 4) / 3600.0;
        if (!MCP_Batteryfile_1.flag && !MCP_Batteryfile_2.flag) {
            batteryfile = (struct_BatteryFile*) & MCP_Batteryfile_1;
            batteryfile->flag = 1;
        }
    }
    if (flags.ST_charge) {
        Battery_SetCurrentBattery(&ST_struct);
        Battery_DoCharge();
        bs->electric_charge += ((bs->current / 21.8) + 7) / 3600.0;
        if (!ST_Batteryfile_1.flag && !ST_Batteryfile_2.flag) {
            batteryfile = (struct_BatteryFile*) & ST_Batteryfile_1;
            batteryfile->flag = 1;
        }
    }
}