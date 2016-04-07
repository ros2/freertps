#include <stdio.h>
#include "pin.h"

#define PORTA_ETH_REFCLK 1
#define PORTA_ETH_CRSDV  7
#define PORTC_ETH_RXD0   4
#define PORTC_ETH_RXD1   5
#define PORTG_ETH_RXER   2

#define PORTG_ETH_TXEN  11
#define PORTG_ETH_TXD0  13
#define PORTG_ETH_TXD1  14

#define PORTC_ETH_MDC    1
#define PORTA_ETH_MDIO   2


#define AF_ENET 11

// PHY: LAN8742A-CZ

void enet_mac_init_pins(void)
{
  printf("enet_mac_init_pins()\r\n");

  pin_set_alternate_function(GPIOA, PORTA_ETH_REFCLK, AF_ENET);
  pin_set_alternate_function(GPIOA, PORTA_ETH_MDIO  , AF_ENET);
  pin_set_alternate_function(GPIOA, PORTA_ETH_CRSDV , AF_ENET);

  pin_set_alternate_function(GPIOG, PORTG_ETH_TXEN  , AF_ENET);
  pin_set_alternate_function(GPIOG, PORTG_ETH_TXD0  , AF_ENET);
  pin_set_alternate_function(GPIOG, PORTG_ETH_TXD1  , AF_ENET);

  pin_set_alternate_function(GPIOC, PORTC_ETH_MDC   , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD0  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD1  , AF_ENET);

  pin_set_output_speed(GPIOG, PORTG_ETH_TXEN, 3); // max beef
  pin_set_output_speed(GPIOG, PORTG_ETH_TXD0, 3); // max beef
  pin_set_output_speed(GPIOG, PORTG_ETH_TXD1, 3); // max beef

  // some boards need a reset pulse... if so, generate one now
  /*
  pin_set_output_state(GPIOA, PORTA_PHY_RESET, 1);
  delay_ms(100);
  pin_set_output_state(GPIOA, PORTA_PHY_RESET, 0);
  delay_ms(200);
  pin_set_output_state(GPIOA, PORTA_PHY_RESET, 1);
  delay_ms(500);
  */
}
