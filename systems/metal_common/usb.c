#include "metal/usb.h"
#include <stdio.h>
#include <string.h>

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
  1, // default: PID one
  METAL_USB_BCD_DEV,
  METAL_USB_VENDOR_STRING,
  METAL_USB_PRODUCT_STRING,
  METAL_USB_NO_SERIAL_STR,
  1 // one configuration
}; 

extern const struct usb_device_desc g_usb_device_desc __attribute__((weak, alias("g_usb_default_device_desc")));

////////////////////////////////////////////////////////////////

// todo: add OTG descriptor. may be useful to say "i'm not OTG" (?)

const struct usb_config_desc g_usb_default_config_desc =
{
  9, // length of this configuration descriptor
  METAL_USB_DESC_TYPE_CONFIG,
  9 + 9 + 2 * sizeof(struct usb_ep_desc), // total length
  1, // number of interfaces
  1, // configuration value
  0, // no string
  0x80, // attributes
  50, // max power
  {
    {
      9, // length of this interface descriptor
      METAL_USB_DESC_TYPE_IFACE,
      0, // interface number
      0, // alternate setting (?)
      2, // endpoints
      0xff, // custom class
      0xff, // custom subclass
      0xff, // custom protocol
      0, // no string
      {
        {
          sizeof(struct usb_ep_desc), 
          METAL_USB_DESC_TYPE_ENDPOINT,
          0x81, // EP1 IN
          METAL_USB_EP_TYPE_BULK, // bulk endpoint
          512,  // max packet size
          1 // interval. ignored for bulk endpoints.
        },
        {
          sizeof(struct usb_ep_desc), 
          METAL_USB_DESC_TYPE_ENDPOINT,
          0x02, // EP2 out
          METAL_USB_EP_TYPE_BULK, // bulk endpoint
          512,  // max packet size
          1 // interval. ignored for bulk endpoints.
        }
      }
    }
  }
};

extern const struct usb_config_desc g_usb_config_desc __attribute__((weak, alias("g_usb_default_config_desc")));

////////////////////////////////////////////////////////////////

const struct usb_lang_list_desc g_usb_default_lang_list_desc = 
{
  sizeof(struct usb_lang_list_desc),
  METAL_USB_DESC_TYPE_STRING,
  {
    METAL_USB_LANG_ID_ENGLISH_USA
  }
};

extern const struct usb_lang_list_desc g_usb_lang_list_desc __attribute__((weak, alias("g_usb_default_lang_list_desc")));

////////////////////////////////////////////////////////////////

const char *g_metal_usb_default_vendor_str = "OSRF";
const char *g_metal_usb_default_product_str = "Widget";

extern const char *g_metal_usb_vendor_str __attribute__((weak, alias("g_metal_usb_default_vendor_str")));
extern const char *g_metal_usb_product_str __attribute__((weak, alias("g_metal_usb_default_product_str")));

static uint8_t g_metal_usb_setup_pkt_buf[256];

////////////////////////////////////////////////////////////////

uint8_t usb_stuff_desc_string(const char *str)
{
  const uint8_t len = strlen(str);
  g_metal_usb_setup_pkt_buf[0] = 2 + 2 * len;
  g_metal_usb_setup_pkt_buf[1] = METAL_USB_DESC_TYPE_STRING;
  for (int i = 0; i < len; i++)
  {
    g_metal_usb_setup_pkt_buf[2 + i*2] = str[i];
    g_metal_usb_setup_pkt_buf[3 + i*2] = 0;
  }
  return 2 + 2 * len;
}

void usb_rx_setup(const uint8_t *buf, const unsigned len)
{
  const uint8_t  req_type  = buf[0];
  const uint8_t  req       = buf[1];
  const uint16_t req_val   = buf[2] | (buf[3] << 8);
  const uint16_t req_index = buf[4] | (buf[5] << 8);
  const uint16_t req_count = buf[6] | (buf[7] << 8);

  //printf("ep0 setup type %02x req %02x val %04x index %04x count %04x\r\n",
  //    req_type, req, req_val, req_index, req_count);
  if (req_type == 0x80 && req == 0x06) // get descriptor
  {
    const void *p_desc = NULL;
    uint16_t desc_len = 0;
    if (req_val == 0x0100) // get device descriptor
    {
      p_desc = &g_usb_device_desc;
      desc_len = sizeof(g_usb_device_desc);
    }
    else if (req_val == 0x0200) // get configuration descriptor
    {
      p_desc = &g_usb_config_desc;
      desc_len = sizeof(g_usb_config_desc);
    }
    else if (req_val == 0x0300) // get string language list
    {
      p_desc = &g_usb_lang_list_desc;
      desc_len = sizeof(g_usb_lang_list_desc);
    }
    else if (req_val == 0x0302) // get product string
    {
      p_desc = g_metal_usb_setup_pkt_buf;
      desc_len = usb_stuff_desc_string(g_metal_usb_product_str);
    }
    else if (req_val == 0x0301) // get vendor string
    {
      p_desc = g_metal_usb_setup_pkt_buf;
      desc_len = usb_stuff_desc_string(g_metal_usb_vendor_str);
    }
    ////////////////////
    if (p_desc)
    {
      int tx_len = desc_len < req_count ? desc_len : req_count;
      usb_tx(0, p_desc, tx_len);
    }
    else
    {
      printf("TRAP!!! unknown descriptor request: 0x%04x\r\n", req_val);
      while(1) { } // IT'S A TRAP
    }
  }
  else if (req_type == 0x00 && req == 0x05) // set address
    usb_set_addr((uint8_t)req_val);
  else if (req_type == 0x00 && req == 0x09) // set configuration
  {
    // todo: call into mac-specific function to set up endpoints, etc.
    usb_tx(0, NULL, 0);
  }
  else
  {
    printf("unknown setup rx: req_type = 0x%02x, req = 0x%02x\r\n",
        req_type, req);
    printf("trapping...\r\n");
    while(1) { } // IT'S A TRAP
  }
}
