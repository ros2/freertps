#ifndef SENSOR_CAMERA_H
#define SENSOR_CAMERA_H

typedef void (*image_cb_t)();

typedef enum{CAMERA_RESOLUTION_VGA,CAMERA_RESOLUTION_QVGA,CAMERA_RESOLUTION_QQVGA} camera_resolution_t;
typedef enum{CAMERA_MODE_SNAPSHOT,CAMERA_MODE_CONTINUOUS} camera_mode_t;
typedef enum{DATA_8_BITS,DATA_10_BITS,DATA_12_BITS,DATA_14_BITS} dcmi_data_width_t;

#endif
