#ifndef SENSOR_CAMERA_H
#define SENSOR_CAMERA_H

#define BUFFER_SIZE 0x2850
static uint32_t aDST_Buffer[BUFFER_SIZE];

typedef void (*image_cb_t)();

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
#endif
