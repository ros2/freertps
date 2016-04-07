#include <stdio.h>
#include "pin.h"

#define PORTA_USB_FS_VBUS  9
#define PORTA_USB_FS_ID   10
#define PORTA_USB_FS_DM   11
#define PORTA_USB_FS_DP   12

#define PORTF_USB_FS_OVERCURRENT   11
#define PORTH_USB_FS_PWR_SWITCH_ON  5

#define AF_USB_FS 10

void usb_fs_init_pins(void)
{
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_DP, AF_USB_FS);
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_DM, AF_USB_FS);
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_ID, AF_USB_FS);
  
  pin_set_output_speed(GPIOA, PORTA_USB_FS_DM, 3);
  pin_set_output_speed(GPIOA, PORTA_USB_FS_DP, 3);
}
