#include <stdio.h>
#include "actuators/led.h"
#include <stdint.h>
#include "freertps/freertps.h"
#include "freertps/timer.h"
#include <string.h>

// todo: auto-generate all of this nonsense
typedef struct
{
  int32_t sec;
  uint32_t nanosec;
} __attribute__((packed)) builtin_interfaces__time_t;

typedef struct
{
  builtin_interfaces__time_t stamp;
  uint32_t frame_id_len_;
  char frame_id[];
} __attribute__((packed)) std_interfaces__header_t;

frudp_pub_t *g_pub = NULL;

#define WIDTH 376
#define HEIGHT 240
static uint8_t img_line[WIDTH];

static uint8_t tx_block[2048]; // use 1k blocks; may need to spill to another
static uint32_t tx_block_wpos;

bool enqueue_block(const uint8_t *block, const uint16_t block_len)
{
  if (block_len > 1024)
  {
    printf("block too big: %d\n", (int)block_len);
    return false;
  }
}

void timer_cb()
{
  if (!g_pub)
    return;

  uint8_t fake_image[WIDTH*HEIGHT*3] = {0};
  for (int row = 0; row < HEIGHT; row++)
    for (int col = 0; col < WIDTH; col++)
    {
      fake_image[row * 3 * WIDTH + col * 3 + 0] = col * (256/WIDTH);
      fake_image[row * 3 * WIDTH + col * 3 + 1] = col * (256/WIDTH);
      fake_image[row * 3 * WIDTH + col * 3 + 2] = 255 - col * (256/WIDTH);
    }

  char __attribute__((aligned(4))) msg[2048] = {0};
  std_interfaces__header_t *header = (std_interfaces__header_t *)msg;
  header->stamp.sec = 1234;
  header->stamp.nanosec = 5678;
  static const char *frame_id = "cam_frame12";
  header->frame_id_len_ = strlen(frame_id) + 1;
  memcpy(header->frame_id, frame_id, header->frame_id_len_);
  uint8_t *wpos = (uint8_t *)(header->frame_id) + header->frame_id_len_; // + 1;
  // next, copy in the width and height
  uint32_t width = WIDTH, height = HEIGHT;
  memcpy(wpos, &width, 4);
  wpos += 4;
  memcpy(wpos, &height, 4);
  wpos += 4;
  const char *encoding = "bgr8";
  uint32_t encoding_len = strlen(encoding) + 1;
  memcpy(wpos, &encoding_len, 4);
  wpos += 4;
  strncpy((char *)wpos, encoding, strlen(encoding) + 1);
  wpos += strlen(encoding) + 1;
  uint8_t bigendian = 0;
  memcpy(wpos, &bigendian, 1);
  wpos += 1;
  // get back to 32-bit alignment
  if ((uint64_t)wpos & 0x3)
    wpos += 4 - ((uint64_t)wpos & 0x3);
  uint32_t step = WIDTH * 3;
  memcpy(wpos, &step, 4);
  wpos += 4;
  uint32_t data_size = WIDTH * HEIGHT * 3;
  memcpy(wpos, &data_size, 4);
  wpos += 4;
  memcpy(wpos, fake_image, WIDTH * HEIGHT * 3);
  //memset(wpos, 0, WIDTH * HEIGHT * 3);
  wpos += WIDTH * HEIGHT * 3;
  uint32_t cdr_len = wpos - (uint8_t *)msg;
  printf("publishing %d bytes\r\n", (int)cdr_len);
  freertps_publish(g_pub, (uint8_t *)msg, cdr_len);
}

int main()
{
  printf("cam main()\r\n");
  freertps_system_init();
  freertps_timer_set_freq(1, timer_cb);
  g_pub = freertps_create_pub("image", "sensor_msgs::msg::dds_::Image_");
  frudp_disco_start();
  while (freertps_system_ok())
  {
    frudp_listen(500000);
    frudp_disco_tick();
  }
  frudp_fini();
  return 0;
}
