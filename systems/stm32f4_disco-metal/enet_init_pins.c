#include <stdio.h>
#include "pin.h"
#include "metal/delay.h"

#define PORTA_ETH_REFCLK 1
#define PORTE_PHY_RESET  2

#define PORTA_ETH_CRSDV  7
#define PORTC_ETH_RXD0   4
#define PORTC_ETH_RXD1   5

#define PORTB_ETH_TXEN  11
#define PORTB_ETH_TXD0  12
#define PORTB_ETH_TXD1  13

#define PORTC_ETH_MDC    1
#define PORTA_ETH_MDIO   2

#define AF_ENET 11

// PHY: LAN8720

void enet_mac_init_pins(void)
{
  printf("enet_mac_init_pins()\r\n");

  pin_set_alternate_function(GPIOA, PORTA_ETH_REFCLK, AF_ENET);
  pin_set_alternate_function(GPIOA, PORTA_ETH_MDIO  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_MDC   , AF_ENET);

  pin_set_alternate_function(GPIOB, PORTB_ETH_TXEN  , AF_ENET);
  pin_set_alternate_function(GPIOB, PORTB_ETH_TXD0  , AF_ENET);
  pin_set_alternate_function(GPIOB, PORTB_ETH_TXD1  , AF_ENET);

  pin_set_alternate_function(GPIOA, PORTA_ETH_CRSDV , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD0  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD1  , AF_ENET);

  pin_set_output_speed(GPIOB, PORTB_ETH_TXEN, 3); // max beef
  pin_set_output_speed(GPIOB, PORTB_ETH_TXD0, 3); // max beef
  pin_set_output_speed(GPIOB, PORTB_ETH_TXD1, 3); // max beef

  pin_set_output(GPIOE, PORTE_PHY_RESET, 1);
  delay_ms(100);
  pin_set_output_state(GPIOE, PORTE_PHY_RESET, 0);
  delay_ms(100);
  pin_set_output_state(GPIOE, PORTE_PHY_RESET, 1);
  delay_ms(100);
}
