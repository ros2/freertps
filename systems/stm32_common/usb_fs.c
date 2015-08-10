#include "usb_fs.h"
#include "peripheral/usb.h"
#include <stdbool.h>

static const usb_config_desc_t g_usb_config_desc =
{
  9, // length of this configuration descriptor
  2, // configuration descriptor type
  9 + 9 + 2 * 7, // total length
  1, // number of interfaces
  0, // configuration value
  0, // no string
  0x80, // attributes
  50, // max power
  {
    {
      9, // length of this interface descriptor
      4, // interface descriptor type
      0, // interface number
      0, // alternate setting
      2, // no extra endpoints
      0xff, // custom class code,
      0xff, // custom subclass code,
      0xff, // custom protocol code
      0, // no string
      {
        {
          7, // length of this endpoint descriptor
          5, // endpoint descriptor type
          0x81, // EP1 IN
          0x02, // bulk endpoint
          64, // max packet size
          1 // interval. ignored for bulk endpoints anyway.
        },
        {
          7, // length of this endpoint descriptor
          5, // endpoint descriptor type
          0x02, // EP2 OUT
          0x02, // bulk endpoint
          64, // max packet size
          1 // interval. ignored for bulk endpoints anyway.
        }
      }
    }
  }
};


void usb_fs_init(){

  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;                    // activate peripheral clock
  usb_fs_init_pins();
  //USB OTG Instance is USB_OTG_FS

  /* Core initialization */
  USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT   |//;      // Enable interrupts
  //TODO only non periodic is needed I think other one shouldnt matter
                         USB_OTG_GAHBCFG_TXFELVL|         // interrupt on Periodic TX Fifo empty
                         USB_OTG_GAHBCFG_PTXFELVL;        // Interrupt on Non periodic FIFO empty

  USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD ;         // Force device mode
  USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD |          //; // Idem
                         ((13 << 10) & USB_OTG_GUSBCFG_TRDT);  // TRDT = 4 * (AHB Clock Frequency / USB clock frequency) + 1, in our case TRDT = 4*(168/48) + 1 = 13
  //FIXME Need for a TOCAL value ?? Will add it if needed
  // No HNP nor SRP, will always be a device and VBUS always be 5V
  USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_OTGINT |
                         USB_OTG_GINTMSK_MMISM  ;


static USB_OTG_DeviceTypeDef * const g_usbd =
  (USB_OTG_DeviceTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE +
                            USB_OTG_DEVICE_BASE);
  /* Device initialization */
  g_usbd->DCFG |= USB_OTG_DCFG_DSPD | //; // use internal PHY at full speed
                      USB_OTG_DCFG_NZLSOHSK; // send STALL handshake
  // unmask interrupts
  USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_ESUSPM   | // Early suspend
                         USB_OTG_GINTMSK_USBSUSPM | // USB suspend
                         USB_OTG_GINTMSK_USBRST   | // Reset
                         USB_OTG_GINTMSK_ENUMDNEM | // Enum done
                         USB_OTG_GINTMSK_SOFM     ; // Start of Frame INT
  while((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_USBRST) != USB_OTG_GINTSTS_USBRST); // wait for reset
  while((USB_OTG_FS->GINTSTS &  USB_OTG_GINTSTS_ENUMDNE) != USB_OTG_GINTSTS_ENUMDNE); // wait for end of enumeration
  uint8_t enum_speed = (g_usbd->DSTS & USB_OTG_DSTS_ENUMSPD)>>1;
  //TODO Program the MPSIZ field in OTG_DIEPCTL0 to set the maximum packet size.

  //The maximum packet size for a control endpoint depends on the enumeration speed. ??? How do we determine packet size accordign to speed ?
  //Lets put 64 for FS and 512 for HS
//  g_usbd->DIEPCTL0 &= ~0x03; // put 2 last bits to 0 (64bits)

  USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM   ; // enable receive fifo non empty interrupts

  //TODO endpoint init + activation ?

}

//TODO This function is called when RXFLVM interrupt catched:
// if((USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_RXFLVL) != USB_OTG_GINTSTS_RXFLVL)
void read_packet() //FIXME put real prototype later
{
  // Mask interrupt: Here or let user/application do it ?
  USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTMSK_RXFLVLM   ;

/* DEVICE MODE ONLY*/
  if((USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_BCNT) == 0) {
  }// test byte count of received packet
//    return XX;                        // No point in reading FIFO
  else{                               // We got mail !
    //TODO pop data from FIFO

    // Check packet status
    uint8_t pktsts = (USB_OTG_FS->GRXSTSR & USB_OTG_PKTSTS) >> 17;
    bool dpid = (USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_DPID) >> 15;
    uint8_t bcnt = (USB_OTG_FS->GRXSTSR & USB_OTG_GRXSTSP_BCNT) >> 4;
    uint8_t epnum = USB_OTG_GRXSTSP_EPNUM;

    if(pktsts == USB_DEV_PKTSTS_OUT_NAK){
      // GLOBAL OUT NAK: will we use it ? shoud we process it ?
      if(dpid !=0)
        return; //ERROR
      // HERE process OUT NAK
      // Not gonna happen for now
    }
    if(pktsts == USB_DEV_PKTSTS_OUT_DATA_RCV){
      // OUT Data received let's process it
      _process_data_pkt();
    }
    if(pktsts == USB_DEV_PKTSTS_OUT_DATA_TC){
      // OUT Transfer complete, doing everything here or raise the TC interrupt ?
      // Should we send buffer/trigger application code only on complete transfer or on every packet received?
      _process_data_pkt();
    }
    if(pktsts == USB_DEV_PKTSTS_SETUP_TC){
      // Setup done Setup interrupt will be generated
    }
    if(pktsts == USB_DEV_PKTSTS_SETUP_PKT_RCV){
      // Setup packet received
      _process_setup_pkt();
    }
  }
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM   ; // Unmask interrupt
}

void _process_setup_pkt(){
}

__attribute__((weak)) void _process_data_pkt(){

}

void _process_setup_request(uint8_t *buffer,uint16_t byte_cnt){
  usb_setup_request_t* req= buffer;
//  switch(buffer[0]&0x80){ //check direction of the request
  switch(req->request_type&0x80){ //check direction of the request
    case 0x80:                              // Host asking data
      if(req->request == GET_CONFIG){
        break;
      }
      if(req->request == GET_DESCRIPTOR){
        break;
      }
      if(req->request == GET_INTERFACE){
        break;
      }
      if(req->request == GET_STATUS){
          if(req->request_type&0x07 == 0){  //
          }
          if(req->request_type&0x07 == 1){  //Requesting interface status
          }
          if(req->request_type&0x07 == 2){  //Requesting endpoint status
          }
        break;
      }
    case 0x00: // Host giving order
      if(req->request == SET_ADDRESS){
        break;
      }
      if(req->request == SET_CONFIG){
        break;
      }
      if(req->request == SET_DESCRIPTOR){
        break;
      }
      if(req->request == SET_FEATURE){
        break;
      }
      if(req->request == SET_INTERFACE){
        break;
      }
      if(req->request == CLEAR_FEATURE){
        break;
      }
  }
}


/******************************
** Public interface for user **
*******************************/
__attribute__((weak)) void usb_receive_done_cb(uint8_t* data){
  // to be setup by user if he wants
}

bool usb_send_data_to_host(uint8_t* data){

}
