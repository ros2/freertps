#include <stdio.h>
#include "freertps/periph/led.h"
#include <stdint.h>
#include "freertps/freertps.h"
#include "freertps/timer.h"
#include <string.h>
#include "metal/systime.h"
#include "metal/usb.h"
#include "metal/delay.h"

#if 0
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

// todo: generalize and refactor this fragmentation system and put it in
// the generic portable freertps publisher library
#define TX_BLOCK_SIZE 1384
static uint8_t tx_block[TX_BLOCK_SIZE];
static uint32_t tx_block_wpos;
static uint32_t frag_num = 1;
static uint32_t g_msg_len = 0;

bool enqueue_block(const uint8_t *block, const uint16_t block_len)
{
  if (block_len > TX_BLOCK_SIZE)
  {
    printf("block too big: %d\n", (int)block_len);
    *(uint32_t *)0 = 42; // boom goes the dynamite
    return false;
  }
  uint32_t bytes_that_fit = TX_BLOCK_SIZE - tx_block_wpos;
  /*
  printf("blocklen = %d bytes_that_fit = %d\n", 
      (int)block_len,
      (int)bytes_that_fit);
  */
  if (block_len < bytes_that_fit)
  {
    //printf("tx_block_wpos = %d\n", (int)tx_block_wpos);
    memcpy(&tx_block[tx_block_wpos], block, block_len);
    tx_block_wpos += block_len;
  }
  else // we've been passed a block that has to span two tx buffers
  {
    //printf("tx_block_wpos = %d\n", (int)tx_block_wpos);
    memcpy(&tx_block[tx_block_wpos], block, bytes_that_fit);
    frudp_publish_user_msg_frag(g_pub, frag_num, tx_block,
        TX_BLOCK_SIZE, TX_BLOCK_SIZE,
        g_msg_len);
    frag_num++;
    uint32_t bytes_that_didnt_fit = block_len - bytes_that_fit;
    if (bytes_that_didnt_fit)
      memcpy(&tx_block[0], &block[bytes_that_fit], bytes_that_didnt_fit);
    tx_block_wpos = bytes_that_didnt_fit;
  }
  return true;
}

bool send_last_partial_block(void)
{
  //printf("send_last_partial_block: %d\r\n", (int)tx_block_wpos);
  if (!tx_block_wpos)
  {
    frag_num = 1; // reset for next time
    return true; // we just happened to end on a block boundary; already done
  }
  frudp_publish_user_msg_frag(g_pub, frag_num, tx_block, 
      TX_BLOCK_SIZE,
      tx_block_wpos,
      g_msg_len);
  tx_block_wpos = 0;
  frag_num = 1;
  return true;
}

#define WIDTH 376
#define HEIGHT 240
static uint8_t g_img_line[WIDTH*3];

void cam_init_test_image(void)
{
  for (int i = 0; i < WIDTH; i++)
  {
    g_img_line[3*i+0] = i/2;
    g_img_line[3*i+1] = i/2;
    g_img_line[3*i+2] = 255-i/2;
  }
}

bool tx_row(const int row)
{
  return enqueue_block(g_img_line, WIDTH * 3);
}

bool tx_header(void)
{
  uint8_t __attribute__((aligned(4))) msg[1024] = {0};
  // prepend the CDR serialization scheme header
  msg[0] = 0;
  msg[1] = 1;
  msg[2] = 0;
  msg[3] = 0;
  std_interfaces__header_t *header = (std_interfaces__header_t *)&msg[4];
  header->stamp.sec = 1234;
  header->stamp.nanosec = 5678;
  static const char *frame_id = "cam_frame12";
  header->frame_id_len_ = strlen(frame_id) + 1;
  memcpy(header->frame_id, frame_id, header->frame_id_len_);
  uint8_t *wpos = (uint8_t *)(header->frame_id) + header->frame_id_len_; // + 4;
  // next, copy in the width and height
  uint32_t width = WIDTH, height = HEIGHT;
  memcpy(wpos, &height, 4);
  wpos += 4;
  memcpy(wpos, &width, 4);
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
  if ((uint32_t)wpos & 0x3)
    wpos += 4 - ((uint32_t)wpos & 0x3);
  uint32_t step = WIDTH * 3;
  memcpy(wpos, &step, 4);
  wpos += 4;
  uint32_t data_size = WIDTH * HEIGHT * 3;
  memcpy(wpos, &data_size, 4);
  wpos += 4;
  uint32_t header_len = wpos - (uint8_t *)msg;
  g_msg_len = header_len + WIDTH * HEIGHT * 3;
  //printf("image header len: %d\n", (int)header_len);
  // send the header
  return enqueue_block(msg, header_len);
}

#if 0
  uint8_t fake_image[WIDTH*HEIGHT*3] = {0};
  for (int row = 0; row < HEIGHT; row++)
    for (int col = 0; col < WIDTH; col++)
    {
      fake_image[row * 3 * WIDTH + col * 3 + 0] = col * (256/WIDTH);
      fake_image[row * 3 * WIDTH + col * 3 + 1] = col * (256/WIDTH);
      fake_image[row * 3 * WIDTH + col * 3 + 2] = 255 - col * (256/WIDTH);
    }
#endif

void timer_cb(void)
{
  if (!g_pub)
    return;
  volatile uint32_t t_start = systime_usecs();
  if (!tx_header())
    return;
  // now, send the rows
  for (int row = 0; row < HEIGHT; row++)
    if (!tx_row(row))
      return;
  send_last_partial_block();
  volatile uint32_t t_end = systime_usecs();
  printf("sent image: %d usec\r\n", (int)(t_end - t_start));
  
  /*
  memcpy(wpos, fake_image, WIDTH * HEIGHT * 3);
  //memset(wpos, 0, WIDTH * HEIGHT * 3);
  wpos += WIDTH * HEIGHT * 3;
  uint32_t cdr_len = wpos - (uint8_t *)msg;
  printf("publishing %d bytes\r\n", (int)cdr_len);
  freertps_publish(g_pub, (uint8_t *)msg, cdr_len);
  */
}
#endif

int main(void)
{
  printf("cam main()\r\n");
  usb_init();
  __enable_irq();
  //freertps_system_init();
  //SCB_EnableICache();
  //cam_init_test_image();
  //freertps_timer_set_freq(1, timer_cb);
  //g_pub = freertps_create_pub("image", "sensor_msgs::msg::dds_::Image_");
  //frudp_disco_start();
  while (1) //freertps_system_ok())
  {
    delay_ms(200);
    led_toggle();

    /*
    timer_cb();
    frudp_listen(1000000);
    frudp_disco_tick();
    */
  }
  frudp_fini();
  return 0;
}
