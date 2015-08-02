#ifndef CAMERA_H
#define CAMERA_H

#include "dcmi.h"
#include "i2c.h"
//#include "sensors/ov9655_from_ds.h"
#include "ov9655.h"

void dcmi_init();

//typedef enum{CAMERA_RESOLUTION_VGA,CAMERA_RESOLUTION_QVGA,CAMERA_RESOLUTION_QQVGA} camera_resolution_t;
//typedef enum{CAMERA_MODE_SNAPSHOT,CAMERA_MODE_CONTINUOUS} camera_mode_t;
//typedef enum{DATA_8_BITS,DATA_10_BITS,DATA_12_BITS,DATA_14_BITS} dcmi_data_width_t;

void camera_set_framerate();
void camera_set_color();
//void camera_set_resolution(camera_resolution_t resolution);
void camera_set_resolution(uint8_t resolution);
//void camera_set_mode(camera_mode_t mode);


// BSP Functions
void camera_init_pins();  // provided in a board-specific file
void camera_power_up();  // Idem
void camera_power__down();  // Idem
#endif
