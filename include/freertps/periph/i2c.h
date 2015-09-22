#ifndef FREERTPS_I2C_H
#define FREERTPS_I2C_H

#include <stdint.h>
#include <stdbool.h>

bool i2c_init(void *i2c);

bool i2c_read(void *i2c, uint8_t device_addr, 
    uint8_t reg_addr, uint8_t len, uint8_t *buffer);

bool i2c_write(void *i2c, uint8_t device_addr, 
    uint8_t reg_addr, uint8_t len, uint8_t *buffer);

#endif
