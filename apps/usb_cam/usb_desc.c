#include "metal/usb.h"

const struct usb_device_desc g_usb_device_desc =
{
  sizeof(struct usb_device_desc),
  METAL_USB_DESC_TYPE_DEVICE,
  METAL_USB_PROTO,
  METAL_USB_DEV_CLASS_CUSTOM,
  METAL_USB_DEV_SUBCLASS_CUSTOM,
  METAL_USB_DEV_PROTO_CUSTOM,
  METAL_USB_MAX_EP0_PKT_SIZE,
  METAL_USB_VID,
  0x2601,
  METAL_USB_BCD_DEV,
  METAL_USB_NO_MFGR_STR,
  METAL_USB_NO_PROD_STR,
  METAL_USB_NO_SERIAL_STR
};

void f(void) { }
