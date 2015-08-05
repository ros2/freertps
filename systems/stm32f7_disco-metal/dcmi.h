#ifndef DCMI_H
#define DCMI_H

#include "sensors/camera.h"
void dcmi_take_snapshot();
//void dcmi_init(image_cb_t cb);
void dcmi_init(image_cb_t cb, image_cb_t dma_cb);
// dcmi_read_frame();
// IT callbacks
#endif
