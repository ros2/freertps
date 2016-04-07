#ifndef FREERTPS_CAM_H
#define FREERTPS_CAM_H

#include <stdbool.h>

void cam_init(void);      // image_cb_t cb, image_cb_t dma_cb);
extern volatile uint8_t *g_cam_frame_buffer;

typedef void (*cam_image_cb_t)(void);
void cam_set_image_cb(cam_image_cb_t cb);

void cam_start_image_capture(void);
bool cam_image_ready(void);

#if 0
#define BUFFER_SIZE 0x2850
static uint32_t __attribute__((unused)) aDST_Buffer[BUFFER_SIZE];


typedef enum{CAMERA_FRAMERATE_15FPS,CAMERA_FRAMERATE_30FPS} camera_framerate_t;
typedef enum{CAMERA_COLOR_RGB_565,CAMERA_COLOR_RGB_555,CAMERA_COLOR_YUV} camera_colorspace_t;
typedef enum{CAMERA_RESOLUTION_VGA,CAMERA_RESOLUTION_QVGA,CAMERA_RESOLUTION_QQVGA} camera_resolution_t;
typedef enum{CAMERA_MODE_SNAPSHOT,CAMERA_MODE_CONTINUOUS} camera_mode_t;
typedef enum{DATA_8_BITS,DATA_10_BITS,DATA_12_BITS,DATA_14_BITS} dcmi_data_width_t;
typedef struct
{
  camera_colorspace_t colorspace;
  camera_mode_t mode;
  camera_resolution_t resolution;
  camera_framerate_t framerate;
  // add other stuff which should be valid for any camera
} __attribute__((packed)) camera_config_t;

void camera_init(image_cb_t cb, image_cb_t dma_cb);
void camera_reset(void);

// generic function, make them weak ?
void camera_set_framerate(camera_framerate_t framerate);
void camera_set_color(camera_colorspace_t color_format);
void camera_set_resolution(camera_resolution_t resolution);
void camera_set_mode(camera_mode_t mode);
void camera_set_nightmode(uint8_t mode);
void camera_take_snapshot(void);
void camera_increase_brightness(void);
void camera_decrease_brightness(void);
void camera_increase_contrast(void);
void camera_decrease_contrast(void);

// BSP Functions
void camera_power_up(void);  // Idem
void camera_power_down(void);  // Idem
#endif


#endif
