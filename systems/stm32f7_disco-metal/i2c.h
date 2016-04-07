#ifndef I2C_H
#define I2C_H

void i2c_init(I2C_TypeDef *i2c);
//uint8_t i2c_read_byte(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte);
void i2c_read(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t* buffer);
void i2c_write(I2C_TypeDef *i2c, uint16_t DeviceAddr, uint16_t RegAddr, uint8_t RegSizeByte, uint8_t* data);
//void i2c_master_send(void); //TODO define prototype and implement
//void i2c_master_receive(void); 
//void i2c_slave_send(void); // no slave for now
//void i2c_slave_receive(void);


#endif
