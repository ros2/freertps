#ifndef CAMERA_H
#define CAMERA_H

#include "dcmi.h"
#include "i2c.h"
#include "sensors/camera.h"
#include "sensors/ov9655.h"

void camera_set_framerate();
void camera_set_color();
void camera_set_resolution(camera_resolution_t resolution);
//void camera_set_resolution(uint8_t resolution);
void camera_set_mode(camera_mode_t mode);
void camera_init(image_cb_t cb);
void camera_reset();
void camera_take_snapshot();

// BSP Functions
void camera_init_pins();  // provided in a board-specific file
void camera_power_up();  // Idem
void camera_power_down();  // Idem
#endif
