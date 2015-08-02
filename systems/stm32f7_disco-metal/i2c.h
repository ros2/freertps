#ifndef I2C_H
#define I2C_H

void i2c_init(I2C_TypeDef *i2c);
uint8_t i2c_read_data(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte);
void i2c_write_data(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t data);
//void i2c_master_send(); //TODO define prototype and implement
//void i2c_master_receive(); 
//void i2c_slave_send(); // no slave for now
//void i2c_slave_receive();


#endif
