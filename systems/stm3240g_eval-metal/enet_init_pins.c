#include <stdio.h>
#include "pin.h"
#include "metal/delay.h"

// PHY: DP83848CVV over MII

#define PORTA_ETH_REFCLK 1

#define PORTA_ETH_CRSDV  7
#define PORTC_ETH_RXD0   4
#define PORTC_ETH_RXD1   5
#define PORTH_ETH_RXD2   6
#define PORTH_ETH_RXD3   7

#define PORTG_ETH_TXEN  11
#define PORTC_ETH_TXCLK  3
#define PORTG_ETH_TXD0  13
#define PORTG_ETH_TXD1  14
#define PORTC_ETH_TXD2   2
#define PORTB_ETH_TXD3   8

#define PORTC_ETH_MDC    1
#define PORTA_ETH_MDIO   2

#define AF_ENET 11

void enet_mac_init_pins(void)
{
  printf("enet_init_pins()\r\n");

  pin_set_alternate_function(GPIOA, PORTA_ETH_REFCLK, AF_ENET);
  pin_set_alternate_function(GPIOA, PORTA_ETH_MDIO  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_MDC   , AF_ENET);

  pin_set_alternate_function(GPIOG, PORTG_ETH_TXEN  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_TXCLK , AF_ENET);
  pin_set_alternate_function(GPIOG, PORTG_ETH_TXD0  , AF_ENET);
  pin_set_alternate_function(GPIOG, PORTG_ETH_TXD1  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_TXD2  , AF_ENET);
  pin_set_alternate_function(GPIOB, PORTB_ETH_TXD3  , AF_ENET);

  pin_set_alternate_function(GPIOA, PORTA_ETH_CRSDV , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD0  , AF_ENET);
  pin_set_alternate_function(GPIOC, PORTC_ETH_RXD1  , AF_ENET);
  pin_set_alternate_function(GPIOH, PORTH_ETH_RXD2  , AF_ENET);
  pin_set_alternate_function(GPIOH, PORTH_ETH_RXD3  , AF_ENET);

  pin_set_output_speed(GPIOG, PORTG_ETH_TXEN , 3); // max beef
  pin_set_output_speed(GPIOC, PORTC_ETH_TXCLK, 3); // max beef
  pin_set_output_speed(GPIOG, PORTG_ETH_TXD0 , 3); // max beef
  pin_set_output_speed(GPIOG, PORTG_ETH_TXD1 , 3); // max beef
  pin_set_output_speed(GPIOC, PORTC_ETH_TXD2 , 3); // max beef
  pin_set_output_speed(GPIOB, PORTB_ETH_TXD3 , 3); // max beef
}
