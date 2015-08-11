#include "usbd_fs.h"
#include "peripheral/usb.h"
#include <stdbool.h>
#include <stdlib.h>

// application functions
extern void usb_ep1_txf_empty() __attribute__((weak));
extern void usb_ep1_tx_complete() __attribute__((weak));
extern void usb_rx(const uint8_t ep, 
                   const uint8_t *data, 
                   const uint8_t len) __attribute__((weak));

#define USB_TIMEOUT 200000
#define USB_INEP(i)  ((USB_OTG_INEndpointTypeDef *)(( uint32_t)USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + (i)*USB_OTG_EP_REG_SIZE))        
#define USB_OUTEP(i) ((USB_OTG_OUTEndpointTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + (i)*USB_OTG_EP_REG_SIZE))
#define USB_FIFO_BASE ((uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE))
#define USB_FIFO(i) ((uint32_t *)(USB_FIFO_BASE + 1024 * i))
#define USB_WAIT_RET_BOOL(cond) \
  do { \
    int count = 0; \
    do { \
      if ( ++count > USB_TIMEOUT ) \
        return false; \
    } while (cond); \
  } while (0)
static usb_dev_desc_t g_usb_dev_desc;
static usb_config_desc_t g_usb_config_desc;
static bool g_usb_config_complete = false;

//FIXME why ep>=4 everywhere ? why 4 IN adn Out EPs ? we have 4 endpoints but they should be 0s and 1 never more
//FIXME What is happening when receiving imcomplete setup requests ? 
static USB_OTG_DeviceTypeDef * const g_usbd = 
  (USB_OTG_DeviceTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE + 
                            USB_OTG_DEVICE_BASE);


// FIFOs management
bool usb_flush_txfifo(uint32_t fifo)
{
//FIXME why shifting 5 ? should be 6 
//Not important: why waiting for txfflsh ? this bit is not cleared by hardware. should wait 8 to 10 clock cycles
  USB_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_TXFFLSH | (fifo << 5);
  USB_WAIT_RET_BOOL((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == 
                    USB_OTG_GRSTCTL_TXFFLSH);
  return true;
}

bool usb_flush_rxfifo()
{
  USB_OTG_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;
  USB_WAIT_RET_BOOL((USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) ==
                    USB_OTG_GRSTCTL_RXFFLSH);
  return true;
}


// Transactions management
bool usb_tx_stall(const uint8_t ep)
{
  if (ep >=4)//g_usb_config_desc.ifaces[0].num_endpoints)
    return false; // adios amigo
  USB_INEP(ep)->DIEPCTL |= USB_OTG_DIEPCTL_STALL;
  return true;
}

//FIXME shouldnt we check that space available in fifo before filling ? 
bool usb_tx(const uint8_t ep, const uint8_t *payload, const uint8_t payload_len){
  if (ep >= 4) //g_usb_config_desc.ifaces[0].num_endpoints)
    return false;
  if (ep != 0 && !g_usb_config_complete)
    return false; // can't transmit yet
  USB_INEP(ep)->DIEPTSIZ = (1 << 19) | payload_len; // set outbound txlen
  USB_INEP(ep)->DIEPCTL  |= USB_OTG_DIEPCTL_EPENA | // enable endpoint
                            USB_OTG_DIEPCTL_CNAK  ; // clear NAK bit
  uint32_t *fifo = USB_FIFO(ep); // memory-mapped magic...
  for (int word_idx = 0; word_idx < (payload_len + 3) / 4; word_idx++)
    *fifo = *((uint32_t *)&payload[word_idx * 4]); // abomination
  return true;
}

// init functions
void create_descriptors(const uint8_t nbEPIn, const uint8_t nbEPOut,const uint16_t maxPktSize){
  // initialize configuration descriptor
  g_usb_config_desc.length = CONFIG_LENGTH; 
  g_usb_config_desc.descriptor_type = CONFIGURATION;
  g_usb_config_desc.total_length = CONFIGURATION + IFACE_LENGTH + (nbEPOut + nbEPIn) * EP_LENGTH;
  g_usb_config_desc.num_interfaces = 1;
  g_usb_config_desc.config_value = 0;
  g_usb_config_desc.config_string_index = 0;
  g_usb_config_desc.attributes = 0x80;
  g_usb_config_desc.max_power = 250;
  for(uint8_t i=0; i<g_usb_config_desc.num_interfaces;i++){
    g_usb_config_desc.ifaces[i].length = IFACE_LENGTH;
    g_usb_config_desc.ifaces[i].descriptor_type = INTERFACE;
    g_usb_config_desc.ifaces[i].interface_number = i;
    g_usb_config_desc.ifaces[i].alternate_setting = 0;
    g_usb_config_desc.ifaces[i].num_endpoints = (nbEPOut + nbEPIn);
    g_usb_config_desc.ifaces[i].interface_class = 0xFF;
    g_usb_config_desc.ifaces[i].interface_subclass = 0xFF;
    g_usb_config_desc.ifaces[i].interface_protocol = 0xFF;
    g_usb_config_desc.ifaces[i].interface_string_index = 0;
    for(uint8_t j=0; j<nbEPOut; j++){
      g_usb_config_desc.ifaces[i].eps[j].length = EP_LENGTH;
      g_usb_config_desc.ifaces[i].eps[j].descriptor_type = ENDPOINT;
      g_usb_config_desc.ifaces[i].eps[j].endpoint_address = (uint8_t)(EP_OUT_ADDR + j);
      g_usb_config_desc.ifaces[i].eps[j].attributes = BULK_XFER;
      g_usb_config_desc.ifaces[i].eps[j].max_packet_size = maxPktSize;
      g_usb_config_desc.ifaces[i].eps[j].interval = 1; // interval in ms for polling: SHOULD BE CONFIGURED FOR HIGH SPEED
    }
    for(uint8_t j=0; j<nbEPIn; j++){
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].length = EP_LENGTH;
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].descriptor_type = ENDPOINT;
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].endpoint_address = (uint8_t)(EP_IN_ADDR + (nbEPOut-1)+j);
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].attributes = BULK_XFER;
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].max_packet_size = maxPktSize;
      g_usb_config_desc.ifaces[i].eps[(nbEPOut-1)+j].interval = 1; // interval in ms for polling: SHOULD BE CONFIGURED FOR HIGH SPEED
    }
  }
  // now the device descriptor
  g_usb_dev_desc.length = DEVICE_LENGTH;
  g_usb_dev_desc.descriptor_type = DEVICE;
  g_usb_dev_desc.bcdUSB = USB2_0;
  g_usb_dev_desc.device_class = 0xFF;    // no official class
  g_usb_dev_desc.device_sub_class = 0xFF;// Idem
  g_usb_dev_desc.device_protocol = 0xFF; // Idem
  g_usb_dev_desc.max_packet_size_0 = 64; // Always 64 even if datas packet size is bigger !!
  g_usb_dev_desc.idVendor = 0xf055;      // Not real vendor ID yet
  g_usb_dev_desc.idProduct = 0x0126;     // Not real Product ID yet
  g_usb_dev_desc.bcdDevice = 0x00;       // first release
  g_usb_dev_desc.Manufacturer = 0x00;    // no string descriptor
  g_usb_dev_desc.Product = 0x00;         // no string descriptor
  g_usb_dev_desc.SerialNumber = 0x00;    // no string descriptor
  g_usb_dev_desc.NumConfiguration = 0x01;// Always use 1 config. Windows dont check the others!
}

void usbd_fs_init(uint8_t nbEPIn,uint8_t nbEPOut,uint16_t maxPktSize){
  create_descriptors(1,1,64);
  usb_fs_init_pins();
//  printf("%u",g_usb_config_desc.length);
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;                    // activate peripheral clock
  //USB OTG Instance is USB_OTG_FS

  /* Core initialization */
  USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT   |//;      // Enable interrupts
                         USB_OTG_GAHBCFG_TXFELVL;        // Interrupt on Non periodic FIFO empty

  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN     ;
  USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD ;         // Force device mode
  USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD  |          //; // Idem
                         ((13 << 10) & USB_OTG_GUSBCFG_TRDT);  // TRDT = 4 * (AHB Clock Frequency / USB clock frequency) + 1, in our case TRDT = 4*(168/48) + 1 = 13
  //FIXME Need for a TOCAL value ?? Will add it if needed
  // No HNP nor SRP, will always be a device and VBUS always be ON(5V)
  USB_OTG_FS->GINTMSK |= //USB_OTG_GINTMSK_OTGINT | // No OTG
                         USB_OTG_GINTMSK_MMISM  ;


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

  NVIC_SetPriority(OTG_FS_IRQn, 8); //FIXME low priority for now should be adjusted according to camera etc for end application
  NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_reset(){
  //set NAK for all OUT endpoints
//  g_usbd->DOEPCTL0 |= USB_OTG_DOEPCTL_SNAK;
  
  // unmask interrupts
  g_usbd->DAINTMSK |= 0x0101; // Unmask INEP 0 and OUTEP 0
  g_usbd->DOEPMSK |= USB_OTG_DOEPMSK_STUPM | USB_OTG_DOEPMSK_XFRCM;
  g_usbd->DIEPMSK |= USB_OTG_DIEPMSK_XFRCM | USB_OTG_DIEPMSK_TOM;
  // setup FIFO RAM for each FIFO (GRXFSIZ and DIEPTXF0)
  
}

__attribute__((weak)) void _process_data_pkt(){
}

bool process_setup_request(uint8_t *buffer,uint16_t byte_cnt){
  usb_setup_request_t* req= buffer;
//  switch(buffer[0]&0x80){ //check direction of the request
  switch(req->request_type&0x80){ //check direction of the request
    case 0x80:                              // Host asking data
      if(req->request == SYNCH_FRAME){
        //Never use ISOCHRONOUS for now
        break;
      }
      if(req->request == GET_CONFIG){
        //assume we never receive this request for now.
        break;
      }
      if(req->request == GET_DESCRIPTOR){
        uint8_t *pdesc = NULL;
        uint8_t desc_len = 0;
        if(req->value == DEVICE){
        pdesc = &(g_usb_dev_desc.length);
        desc_len = DEVICE_LENGTH;
        }
        if(req->value == STRING)
          //dont care we dont use them
        if(req->value == CONFIGURATION){
          pdesc = (uint8_t *)&g_usb_config_desc;
          desc_len = g_usb_config_desc.total_length;
        }
        if(req->value == DEVICE_QUALIFIER){}
          //ONLY FOR HS
        if(req->value == OTHER_SPEED_CONFIG){}
          //ONLY FOR HS
        if (pdesc) // send descriptor to host
        {
          int write_len = req->length < desc_len ? req->length : desc_len;
          //TODO: loop until done sending it, if needed
          return usb_tx(0, pdesc, write_len);
        }
        else
          return usb_tx_stall(0); // we don't know how to handle this request
        break; // should never be reached
      }
      if(req->request == GET_INTERFACE){ // Right now we always have only one interface
        break;
      }
      if(req->request == GET_STATUS){
        uint16_t status;
        if((req->request_type & REQUEST_STATUS_MASK) == 0){       //Requesting Device status
          status =  ((g_usb_config_desc.attributes & 0x40)>>6) |  // self powered attribute
                    ((g_usb_config_desc.attributes & 0x20)>>4);   // remote wakeup
        }
        else if((req->request_type & REQUEST_STATUS_MASK) == 1)  //Requesting interface status
          status = 0;           // always 0
        else if((req->request_type & REQUEST_STATUS_MASK) == 2)  //Requesting endpoint status
          status = 0;           // TODO check if endpoint is halted to set LSb of status.
        else
          status =0;
        return usb_tx(0, &status, sizeof(status)); // send a status packet back
        break;

      } 
    case 0x00: // Host giving order
      if(req->request == SET_ADDRESS){
        g_usbd->DCFG &= ~USB_OTG_DCFG_DAD;
        g_usbd->DCFG |= ((req->value) <<4) & USB_OTG_DCFG_DAD;
        usb_tx(0, NULL, 0);           // send status IN packet as ACK
        printf("set addr %d\r\n", req->value);
        return true;
      }
      if(req->request == SET_CONFIG){
        if (req->value != 0)
          return usb_tx_stall(0);
        // enable the IN endpoint for configuration #0
        USB_INEP(1)->DIEPINT  = USB_OTG_DIEPINT_XFRC; // interrupt on TXF empty
        USB_INEP(1)->DIEPTSIZ = 64; 
        USB_INEP(1)->DIEPCTL = USB_OTG_DIEPCTL_SNAK    | // set NAK bit
                               (1 << 22)               | // use txfifo #1
                               USB_OTG_DIEPCTL_EPTYP_1 | // bulk transfer
                               USB_OTG_DIEPCTL_USBAEP  | // active EP (always 1)
                               USB_OTG_DIEPCTL_SD0PID_SEVNFRM |
                               64; // max packet size

        // enable the OUT endpoint for configuration #0
        USB_OUTEP(2)->DOEPTSIZ = (1 << 19) | 64; // buffer one full-length packet
        USB_OUTEP(2)->DOEPCTL = USB_OTG_DOEPCTL_EPENA  | // enable endpoint
                                USB_OTG_DOEPCTL_USBAEP | // active endpoint (always 1)
                                USB_OTG_DOEPCTL_CNAK   | // clear NAK bit
                                USB_OTG_DOEPCTL_SD0PID_SEVNFRM |
                                USB_OTG_DOEPCTL_EPTYP_1 |
                                64; // max packet size = 64 bytes
        g_usbd->DAINTMSK = 0x40002; // OUT2 and IN1 IRQ . used to be 0x10001
        // done configuring endpoints; now we'll send an empty status packet back
        g_usb_config_complete = true;
        uint8_t bogus[64] = {0};
        usb_tx(1, bogus, sizeof(bogus)); // kick-start the FIFO low interrupt
        return usb_tx(0, NULL, 0);    // return status IN as ACK
      }
      if(req->request == SET_DESCRIPTOR){
        // No way you tell me what my nature is!
        break;
      }
      if(req->request == SET_INTERFACE){// Right now we always have only one interface
        break;
      }
      if(req->request == SET_FEATURE){
        // EP always running and no Remote wakeup for now
        // Always device no OTG for now
        break;
      }
      if(req->request == CLEAR_FEATURE){
        // EP always running for now
        // Always device no OTG for now
        break;
      }
  }
  return false;
}


void otg_fs_vector(){
  //received reset interrupt
  if(USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_USBRST){
    printf("usb reset interrupt\r\n");
    usb_flush_txfifo(0);
    g_usbd->DCFG |= USB_OTG_DCFG_DSPD | //; // use internal PHY at full speed
                    USB_OTG_DCFG_NZLSOHSK; // send STALL handshake 

    for (int i = 0; i < 4; i++)
    {
      USB_OUTEP(i)->DOEPINT = 0xff; // wipe out any pending EP flags
      USB_INEP(i)->DIEPINT = 0xff; // wipe out any pending EP flags
      //USB_OUTEP(i)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
    }
     
    g_usbd->DOEPMSK |= USB_OTG_DOEPMSK_STUPM |  // enable setup-done irq
                       USB_OTG_DOEPMSK_EPDM  |  // enable EP-disabled irq
                       USB_OTG_DOEPMSK_XFRCM ;  // enable tx-done irq
    g_usbd->DIEPMSK |= USB_OTG_DIEPMSK_TOM   |  // timeout irq
                       USB_OTG_DIEPMSK_XFRCM |
                       USB_OTG_DIEPMSK_EPDM;

    // set up FIFO sizes, in bytes:
    #define USB_RXFIFO_SIZE     512
    #define USB_TXFIFO_EP0_SIZE 128
    #define USB_TXFIFO_EP1_SIZE 128
     
    USB_OTG_FS->GRXFSIZ = USB_RXFIFO_SIZE / 4; // size is in 32-bit words !
    g_usbd->DCFG &= ~USB_OTG_DCFG_DAD; // zero out the device address fields
    uint32_t usb_ram_addr = USB_RXFIFO_SIZE;
    // the length of this buffer is in 32-bit words, but addr is bytes. argh.
    USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = ((USB_TXFIFO_EP0_SIZE/4) << 16) |
                                     usb_ram_addr;
    usb_ram_addr += USB_TXFIFO_EP0_SIZE;
    // the length of this buffer is in 32-bit words, but addr is bytes. argh.
    USB_OTG_FS->DIEPTXF[0] = ((USB_TXFIFO_EP1_SIZE/4) << 16) | 
                             usb_ram_addr;
    // EP config and Activation
    USB_INEP (0)->DIEPTSIZ = 64;
    USB_OUTEP(0)->DOEPTSIZ = (USB_OTG_DOEPTSIZ_PKTCNT & (1 << 19)) |
                             USB_OTG_DOEPTSIZ_STUPCNT | // allow 3 setup pkt
                             (3 * 8); // transfer size
    USB_INEP (0)->DIEPCTL = USB_OTG_DIEPCTL_USBAEP | // active EP (always 1)
                            USB_OTG_DIEPCTL_SNAK   ; // set NAK bit
    USB_OUTEP(0)->DOEPCTL = USB_OTG_DOEPCTL_EPENA  | // enable endpoint
                            USB_OTG_DOEPCTL_USBAEP | // active EP (always 1)
                            USB_OTG_DOEPCTL_CNAK   ; // clear NAK bit

    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_USBRST; // clear the flag (rc_w1)
  }
  //enumeration done interrupt
  else if(USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_ENUMDNE){
    //TODO Should initiate endpoints according to enum speed ?
    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_ENUMDNE; // clear the flag (rc_w1)
  }
  else if (USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_SOF)
  {
    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_SOF; // clear the flag (rc_w1)
  }
  else if (USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_IEPINT)
  {
    //printf("iepint vec = 0x%08x\r\n", (unsigned)g_usbd->DAINT);
    //if (usb_ep1_txf_empty)
    //  usb_ep1_txf_empty();

    if (g_usbd->DAINT & 0x2)
    {
      // EP1 is firing an IRQ
      if (USB_INEP(1)->DIEPINT & USB_OTG_DIEPINT_XFRC)
      {
        USB_INEP(1)->DIEPINT = USB_OTG_DIEPINT_XFRC; // clear the flag
        // fire the handler
        if (usb_ep1_tx_complete)
          usb_ep1_tx_complete();
      }
    }
  }
  else if(USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_RXFLVL){
    USB_OTG_FS->GINTMSK &= ~USB_OTG_GINTMSK_RXFLVLM   ;
  
  /* DEVICE MODE ONLY*/

    static uint8_t setup_buf[64] = {0};
    static uint8_t setup_nbytes = 0;
    const volatile uint32_t rx_status = USB_OTG_FS->GRXSTSP;
    // Check packet status
    uint8_t pktsts = (rx_status & USB_OTG_PKTSTS) >> 17;
    bool dpid = (rx_status & USB_OTG_GRXSTSP_DPID) >> 15;
    uint8_t nbytes = (rx_status & USB_OTG_GRXSTSP_BCNT) >> 4;
    uint8_t epnum = rx_status & USB_OTG_GRXSTSP_EPNUM;
    if (epnum >= 4)
    {
      printf("woah there partner. epnum = %d\r\n", (int)epnum);
      return;
    }
    uint16_t limit;
    if(epnum == 0)
      limit=64;
    else
      limit = g_usb_config_desc.ifaces[0].eps[epnum-1].max_packet_size;
    if (nbytes > limit)
    {
      printf("woah there partner. nbytes = %d\r\n", (int)nbytes);
      return;
    }
    uint8_t buf[72] ={0};
    int wpos = 0;
    uint32_t w = 0;
    for (int rword = 0; rword < nbytes/4; rword++)
    {
      w = *USB_FIFO(epnum);
      buf[wpos++] =  w        & 0xff;
      buf[wpos++] = (w >>  8) & 0xff;
      buf[wpos++] = (w >> 16) & 0xff;
      buf[wpos++] = (w >> 24) & 0xff;
    }
    if (nbytes % 4)
    {
      w = *USB_FIFO(epnum);
      for (int partial = 0; partial < nbytes % 4; partial++)
      {
        buf[wpos++] = w & 0xff;
        w >>= 8;
      }
    }
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
      //FIXME
      // Should we send *buffer/trigger application code only on complete transfer or on every packet received?
      _process_data_pkt();
    }
    if(pktsts == USB_DEV_PKTSTS_SETUP_TC){
      process_setup_request(setup_buf, setup_nbytes);
      USB_OUTEP(0)->DOEPCTL = USB_OTG_DOEPCTL_EPENA  | // enable endpoint
                              USB_OTG_DOEPCTL_USBAEP | // active endpoint (?)
                              USB_OTG_DOEPCTL_CNAK   ; // clear NAK bit
      // Setup done Setup interrupt will be generated and ignored?
    }
    if(pktsts == USB_DEV_PKTSTS_SETUP_PKT_RCV){
      // Setup packet received
      memcpy(setup_buf, buf, nbytes);
      setup_nbytes = nbytes;
    }
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM   ; // Unmask interrupt
  }
}

