#include "metal/usb.h"
#include <stdio.h>

void usb_init()
{
  printf("usb_init()\r\n");
  PMC->PMC_PCER1 |= 1 << (ID_USBHS - 32);
  USBHS->USBHS_CTRL = 
    USBHS_CTRL_UIMOD | // device mode
    USBHS_CTRL_USBE  ; // enable USB
  PMC->CKGR_UCKR =
    CKGR_UCKR_UPLLEN        | // enable usb pll
    CKGR_UCKR_UPLLCOUNT(0xf); // set lock time
  while(!(PMC->PMC_SR & PMC_SR_LOCKU)); // wait for usb pll lock
  PMC->PMC_USB = 
    PMC_USB_USBS      | // select UPLL for input for FS USB clock (48 MHz)
    PMC_USB_USBDIV(9) ; // FSUSB clock is 480 Mhz / (9 + 1) = 48 MHz
  USBHS->USBHS_DEVIER = USBHS_DEVIER_EORSTES; // enable end-of-reset interrupt
  NVIC_SetPriority(USBHS_IRQn, 1);
  NVIC_EnableIRQ(USBHS_IRQn);
  USBHS->USBHS_DEVCTRL &= !USBHS_DEVCTRL_DETACH; // enable usb output pads
}

static void usb_reset_ep0()
{
  USBHS->USBHS_DEVEPT |=  USBHS_DEVEPT_EPRST0;
  USBHS->USBHS_DEVEPT &= ~USBHS_DEVEPT_EPRST0; // bring EP0 out of reset
  USBHS->USBHS_DEVIER = USBHS_DEVIER_PEP_0; // enable interrupt for EP0
  USBHS->USBHS_DEVEPTIER[0] = USBHS_DEVEPTIER_RXSTPES;
  USBHS->USBHS_DEVEPT |= USBHS_DEVEPT_EPEN0; // enable endpoint
  USBHS->USBHS_DEVEPTCFG[0] = USBHS_DEVEPTCFG_EPSIZE(3); // EP0 is 64-byte
  USBHS->USBHS_DEVEPTCFG[0] |= USBHS_DEVEPTCFG_ALLOC; // alloc some dpram
  //printf("ep0 status: 0x%08x\r\n", (unsigned)USBHS->USBHS_DEVEPTISR[0]);
}

static void usb_handle_setup_request(uint8_t *req_pkt)
{
  const uint8_t  req_type  = req_pkt[0];
  const uint8_t  req       = req_pkt[1];
  const uint16_t req_val   = req_pkt[2] | (req_pkt[3] << 8);
  const uint16_t req_index = req_pkt[4] | (req_pkt[5] << 8);
  const uint16_t req_count = req_pkt[6] | (req_pkt[7] << 8);

  printf("ep0 setup type %02x req %02x val %04x index %04x count %04x\r\n",
      req_type, req, req_val, req_index, req_count);
  if (req_type == 0x80 && req == 0x06) // get descriptor
  {
    if (req_val == 0x0100) // get device descriptor
    {
      // todo: transmit device descriptor
      printf("TODO: transmit device descriptor\r\n");
    }
  }
}

void usbhs_vector()
{
  //printf("usbhs_vector()\r\n");
  if (USBHS->USBHS_DEVISR & USBHS_DEVISR_EORST)
  {
    USBHS->USBHS_DEVICR = USBHS_DEVICR_EORSTC; // clear end-of-reset bit
    usb_reset_ep0();
    //printf("usb general status: 0x%08x\r\n", (unsigned)USBHS->USBHS_SR);
  }
  else if (USBHS->USBHS_DEVISR & USBHS_DEVISR_PEP_0)
  {
    printf("ep0 irq\r\n");
    if (USBHS->USBHS_DEVEPTISR[0] & USBHS_DEVEPTISR_RXSTPI)
    {
      uint32_t setup_pkt[2];
      volatile uint32_t *ep0_fifo = (volatile uint32_t *)USBHS_RAM_ADDR;
      setup_pkt[0] = *ep0_fifo;
      memory_sync();
      setup_pkt[1] = *ep0_fifo;
      memory_sync();
      usb_handle_setup_request((uint8_t *)&setup_pkt[0]);
      USBHS->USBHS_DEVEPTICR[0] = USBHS_DEVEPTICR_RXSTPIC; // clear irq flag
    }
    else
      printf("unknown ep0 irq. ep0 status = 0x%08x\r\n",
          (unsigned)USBHS->USBHS_DEVEPTISR[0]);
    while(1); // trap!
  }
  else
  {
    printf("unhandled usbhs vector: 0x%08x\r\n",
        (unsigned)USBHS->USBHS_DEVISR);
    while (1); // it's a trap
  }
}

