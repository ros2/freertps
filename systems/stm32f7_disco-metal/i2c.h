#ifndef I2C_H
#define I2C_H

void i2c_init();
void i2c_request_data(uint16_t DeviceAddr, uint16_t RegAddr);
void i2c_master_send(); //TODO define prototype and implement
//void i2c_master_receive(); 
//void i2c_slave_send(); // no slave for now
//void i2c_slave_receive();


#endif
