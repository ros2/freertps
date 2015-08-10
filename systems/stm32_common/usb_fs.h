#ifndef USB_OTG_FS_H
#define USB_OTG_FS_H 

/*Packet Status*/
/*Device mode*/
#define USB_DEV_PKTSTS_OUT_NAK       0x01
#define USB_DEV_PKTSTS_OUT_DATA_RCV  0x02
#define USB_DEV_PKTSTS_OUT_DATA_TC   0x03
#define USB_DEV_PKTSTS_SETUP_TC      0x04
#define USB_DEV_PKTSTS_SETUP_PKT_RCV 0x06
/*Host mode*/
#define USB_DEV_PKTSTS_IN_DATA_RCV      0x02
#define USB_DEV_PKTSTS_IN_DATA_TC       0x03
#define USB_DEV_PKTSTS_DATA_TOGGLE_ERR  0x05
#define USB_DEV_PKTSTS_CH_HALTED        0x07

/*DATA PID*/
/* Device mode */
#define USB_DPID_DATA0 0
#define USB_DPID_DATA1 2
#define USB_DPID_DATA2 1
#define USB_DPID_MDATA 3

void usb_fs_init();
void usb_fs_init_pins(); // BSP function


#endif
