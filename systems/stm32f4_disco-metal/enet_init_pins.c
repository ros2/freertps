#include <stdio.h>
#include "pin.h"

// TODO: look up pin assignments on STM32f4-disco board

#define PORTA_ETH_REFCLK 1
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

void enet_init_pins()
{
  printf("enet_init_pins()\r\n");

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
