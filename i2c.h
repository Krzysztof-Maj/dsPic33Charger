/* 
 * File:   i2c.h
 * Author: Krzysiek
 *
 * Created on 10 marzec 2016, 13:28
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif
    /* SCL i SDA -- RB8/RP8 (44) & RB9/RP9 in DsPIC33fj128GP804 */
#define uint8 unsigned char
#define MCP9804_G_adr   0b00110000
#define MCP9804_res     12
#define MCP79412_RTC_adr  0b11011110
#define MCP79412_EE_adr 0b10101110
#define MCP79412_vbaten 1       // 1 if battery is insert

    void i2c_INIT(unsigned long br);
    void i2c_write(uint8 data);
    uint8 i2c_read(uint8 acknowledge);
    void i2c_MCP4451_INIT(void);
    void i2c_W0(unsigned int value); // MCP16323T
    void i2c_W0_increment(void);
    void i2c_W0_decrement(void);
    void i2c_W1(unsigned int value); // ST1S10
    void i2c_W1_increment(void);
    void i2c_W1_decrement(void);
    void i2c_MCP9804_init(unsigned char adr, unsigned char resolution);
    int i2c_MCP9804(unsigned char adr);
    void i2c_MCP79412_init(void);
    void i2c_MCP79412_setTime(void);
    void i2c_MCP79412_setDate(void);
    void i2c_MCP79412_readTime(void);
    void i2c_MCP79412_readDate(void);

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

