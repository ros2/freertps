#include <stdio.h>
#include "pin.h"

#define PORTD_USB_FS_OVERCURRENT 4
#define PORTD_USB_FS_PWR_SWITCH_ON 5
#define PORTA_USB_FS_ID 10
#define PORTA_USB_FS_DM 11
#define PORTA_USB_FS_DP 12
#define PORTJ_USB_FS_VBUS 12

#define AF_USB_FS 10

void usb_fs_init_pins(){
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_DP, AF_USB_FS);
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_DM, AF_USB_FS);
  pin_set_alternate_function(GPIOA, PORTA_USB_FS_ID, AF_USB_FS);
 
  // Output type ? Push Pull ok? 
//  pin_set_output_type(GPIOD, PORTD_USB_FS_OVERCURRENT, PIN_OUTPUT_TYPE_OPEN_DRAIN);
//  pin_set_output_type(GPIOD, PORTD_USB_FS_PWR_SWITCH_ON, PIN_OUTPUT_TYPE_OPEN_DRAIN);
//  pin_set_output_type(GPIOJ, PORTJ_USB_FS_VBUS, PIN_OUTPUT_TYPE_OPEN_DRAIN);

  pin_set_output_speed(GPIOA, PORTA_USB_FS_DP, 3);
  pin_set_output_speed(GPIOA, PORTA_USB_FS_DM, 3);
}
