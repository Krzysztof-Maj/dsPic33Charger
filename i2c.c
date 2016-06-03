#include<p33FJ128GP804.h>
#define FCY 40000000UL
#define uint8 unsigned char
#define ACK 0
#define NACK 1
#include "hardware.h"
struct_RTC_Date RTC_Date = {2,10,5,16};
struct_RTC_Time RTC_Time;

void i2c_INIT(unsigned long br) {
    I2C1BRG = (FCY / br - FCY / 10000000) - 1; // set interface baud rate
    I2C1CONbits.I2CEN = 1; // turn on module
}

void i2c_start(void) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
}

void i2c_stop(void) {
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
}

void i2c_repeatedStart(void) {
    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN);
}

void i2c_write(uint8 data) {
    I2CTRN = data; // last bit (R=0) (W=1)
    while (I2CSTATbits.TRSTAT); // waiting for answer from slave-a
}

uint8 i2c_read(uint8 acknowledge) {
    I2C1CONbits.RCEN = 1;
    while (I2C1CONbits.RCEN);
    I2C1CONbits.ACKDT = acknowledge;
    I2C1CONbits.ACKEN = 1;
    while (I2C1CONbits.ACKEN);
    return (uint8) I2C1RCV;
}

void i2c_MCP4451_INIT(void) {
    i2c_start();
    i2c_write(0b01011000); //0x01011000b potentiometer addr & write (0)
    i2c_write(0x40); // 0x04 is addr TCON0 whitch configure w0 i w1
    i2c_write(0xFF); // turn on w0 & w1
    i2c_stop();
}

void i2c_W0(unsigned int value) {
    i2c_start();
    i2c_write(0b01011000); // potentiometer addr & write
    i2c_write(0x00 | (0x01 & ((uint8) value >> 8))); // addr w0, write command & older 9 bit value
    i2c_write((uint8) value); // the rest of value
    i2c_stop();
}

void i2c_W0_increment(void) {
    i2c_start();
    i2c_write(0b01011000);
    i2c_write(0x04); // addr W0 and increment command (01)
    i2c_stop();
}

void i2c_W0_decrement(void) {
    i2c_start();
    i2c_write(0b01011000);
    i2c_write(0x08); // addre W0 and decrement command (10)
    i2c_stop();
}

void i2c_W1(unsigned int value) {
    i2c_start();
    i2c_write(0b01011000); // potentiometer addr & write
    i2c_write(0x10 | (0x01 & ((uint8) value >> 8))); // addr w1, write command & older 9 bit value
    i2c_write((uint8) value); // the rest of value
    i2c_stop();
}

void i2c_W1_increment(void) {
    i2c_start();
    i2c_write(0b01011000);
    i2c_write(0x14); // addr W0 and increment command (01)
    i2c_stop();
}

void i2c_W1_decrement(void) {
    i2c_start();
    i2c_write(0b01011000);
    i2c_write(0x18); // addre W0 and decrement command (10)
    i2c_stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////* MCP9804 FUNCTION *///////////////////////////////////////////////////////////

void i2c_MCP9804_init(unsigned char adr, unsigned char resolution) {
    i2c_start();
    i2c_write(adr & 0xFE);
    i2c_write(0x01); // chose in pointer (register) config register
    i2c_write(0x00); // chose 0
    i2c_write(0x00); // and once more time 0
    i2c_stop();
    i2c_start();
    i2c_write(adr & 0xFE);
    i2c_write(0x08); // resolution register
    i2c_write(resolution - 9);
    i2c_stop();
}

int i2c_MCP9804(unsigned char adr) {
    int temperature = 0;
    i2c_start();
    i2c_write(adr & 0xFE);
    i2c_write(0x05);
    i2c_repeatedStart();
    i2c_write(adr | 0x01);
    temperature |= (0x1F & i2c_read(ACK)) << 8;
    temperature |= i2c_read(NACK);
    i2c_stop();
    if (0x1000 & temperature) {
        temperature &= 0x0FFF;
        temperature = ~temperature + 1;
    }
    return temperature;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////*  MCP79412 FUNCTION *///////////////////////////////////////////

static unsigned char BCDtoDEC(unsigned char bcd) {
    return ((bcd >> 4)* 10) + (bcd & 0x0F);
}

static unsigned char DECtoBCD(unsigned char dec) {
    return ((dec / 10) << 4) + (dec % 10);
}

void i2c_MCP79412_setTime(void) {
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE); // write
    i2c_write(0x00); // second register
    i2c_write(0x00); // stoped clock (most significant bit (8) in second register)
    i2c_write(DECtoBCD(RTC_Time.minute));
    i2c_write(0x3F & DECtoBCD(RTC_Time.hour));
    i2c_stop();
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x00);
    i2c_write(0x80 | DECtoBCD(RTC_Time.second));
    i2c_stop();
}

void i2c_MCP79412_setDate(void) {
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x03); // addr week day
    i2c_write(MCP79412_vbaten ? (0x08 | RTC_Date.wday) : (0x07 & RTC_Date.wday));
    i2c_write(DECtoBCD(RTC_Date.day));
    i2c_write(DECtoBCD(RTC_Date.month));
    i2c_write(DECtoBCD(RTC_Date.year));
    i2c_stop();
}

void i2c_MCP79412_readTime(void) {
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x00); // addr second
    i2c_repeatedStart();
    i2c_write(MCP79412_RTC_adr | 0x01);
    RTC_Time.second = BCDtoDEC(0x7F & i2c_read(ACK));
    RTC_Time.minute = BCDtoDEC(i2c_read(ACK));
    RTC_Time.hour = BCDtoDEC(i2c_read(NACK));
    i2c_stop();
}

void i2c_MCP79412_readDate(void) {
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x03); // adres week day
    i2c_repeatedStart();
    i2c_write(MCP79412_RTC_adr | 0x01);
    RTC_Date.wday = BCDtoDEC(0x07 & i2c_read(ACK));
    RTC_Date.day = BCDtoDEC(i2c_read(ACK));
    RTC_Date.month = BCDtoDEC(0x1F & i2c_read(ACK));
    RTC_Date.year = BCDtoDEC(i2c_read(NACK));
    i2c_stop();
}

static char MCP79412_InitCheck(void) {
    char tmp;
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x5F);
    i2c_repeatedStart();
    i2c_write(MCP79412_RTC_adr | 0x01);
    tmp = i2c_read(NACK);
    i2c_stop();
    return tmp;
}

void i2c_MCP79412_init(void) {
    if (MCP79412_InitCheck()) return;
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x00); // second register
    i2c_write(0x80); // turn on clock
    i2c_write(0x56); // min 56
    i2c_write(0x12); // 24h - 12:56:00
    i2c_write(0x09); // turn on battery mode
    i2c_write(0x02); // 2
    i2c_write(0x05); // mai
    i2c_write(0x16); // 2016
    i2c_write(0x40); // interrupt 1 Hz
    i2c_stop();
    i2c_start();
    i2c_write(MCP79412_RTC_adr & 0xFE);
    i2c_write(0x5F);
    i2c_write(0x01);
    i2c_stop();
}
