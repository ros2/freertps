#ifndef CAMERA_H
#define CAMERA_H

#include "dcmi.h"
#include "i2c.h"
#include "sensors/camera.h"
#include "sensors/ov9655.h"

//void camera_init(image_cb_t cb);
void camera_init(image_cb_t cb, image_cb_t dma_cb);
void camera_reset();

// generic function, make them weak ?
void camera_set_framerate(camera_framerate_t framerate);
void camera_set_color(camera_colorspace_t color_format);
void camera_set_resolution(camera_resolution_t resolution);
void camera_set_mode(camera_mode_t mode);
void camera_set_nightmode(uint8_t mode);
void camera_take_snapshot();
void camera_increase_brightness();
void camera_decrease_brightness();
void camera_increase_contrast();
void camera_decrease_contrast();


// BSP Functions
void camera_init_pins();  // provided in a board-specific file
void camera_power_up();  // Idem
void camera_power_down();  // Idem
#endif
