#include "metal/usb.h"
#include <stdio.h>

// default supplied for apps that don't need/want usb support
const struct usb_device_desc g_usb_default_device_desc =
{
  sizeof(struct usb_device_desc),
  METAL_USB_DESC_TYPE_DEVICE,
  METAL_USB_PROTO,
  METAL_USB_DEV_CLASS_CUSTOM,
  METAL_USB_DEV_SUBCLASS_CUSTOM,
  METAL_USB_DEV_PROTO_CUSTOM,
  METAL_USB_MAX_EP0_PKT_SIZE,
  METAL_USB_VID,
  0, // default: PID zero
  METAL_USB_BCD_DEV,
  METAL_USB_NO_MFGR_STR,
  METAL_USB_NO_PROD_STR,
  METAL_USB_NO_SERIAL_STR
}; 

extern const struct usb_device_desc g_usb_device_desc __attribute__((weak, alias("g_usb_default_device_desc")));

void usb_rx_setup(const uint8_t *buf, const unsigned len)
{
  const uint8_t  req_type  = buf[0];
  const uint8_t  req       = buf[1];
  const uint16_t req_val   = buf[2] | (buf[3] << 8);
  const uint16_t req_index = buf[4] | (buf[5] << 8);
  const uint16_t req_count = buf[6] | (buf[7] << 8);

  printf("ep0 setup type %02x req %02x val %04x index %04x count %04x\r\n",
      req_type, req, req_val, req_index, req_count);
  if (req_type == 0x80 && req == 0x06) // get descriptor
  {
    if (req_val == 0x0100) // get device descriptor
    {
      printf("req dev desc\r\n");
      usb_tx(0, &g_usb_device_desc, sizeof(g_usb_device_desc));
      //printf("transmit device descriptor\r\n");
    }
    else
    {
      printf("unknown descriptor request: 0x%04x\r\n", req_val);
      while(1) { } // IT'S A TRAP
    }
  }
  else if (req_type == 0x00 && req == 0x05) // set address
  {
    uint8_t addr = (uint8_t)req_val;
    usb_set_addr(addr);
    //while(1) { } // IT'S A TRAP
  }
  else
  {
    printf("unknown setup rx: req_type = 0x%02x, req = 0x%02x\r\n",
        req_type, req);
    printf("trapping...\r\n");
    while(1) { } // IT'S A TRAP
  }
}
