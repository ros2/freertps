#include "pin.h"
#include <stdio.h>

static void pin_enable_pio(Pio *pio)
{
  if (pio == PIOA)
    PMC->PMC_PCER0 |= (1 << ID_PIOA);
  else if (pio == PIOB)
    PMC->PMC_PCER0 |= (1 << ID_PIOB);
  else if (pio == PIOC)
    PMC->PMC_PCER0 |= (1 << ID_PIOC);
  else if (pio == PIOD)
    PMC->PMC_PCER0 |= (1 << ID_PIOD);
  else if (pio == PIOE)
    PMC->PMC_PCER0 |= (1 << ID_PIOE);
}

void pin_set_mux(Pio *pio, const unsigned pin_idx, 
    const unsigned mux_idx)
{
  if (pin_idx > 31 || mux_idx > 3)
    return; // adios amigo
  pin_enable_pio(pio);
  const unsigned pin_mask = 1 << pin_idx;
  // first, clear whatever was there before for this pin
  pio->PIO_ABCDSR[0] &= ~pin_mask;
  pio->PIO_ABCDSR[1] &= ~pin_mask;
  // now , set the necessary bits
  pio->PIO_ABCDSR[0] |= mux_idx & 0x1 ? pin_mask : 0;
  pio->PIO_ABCDSR[1] |= mux_idx & 0x2 ? pin_mask : 0;
  // and disable GPIO control of pin (that is, let the peripheral mux have it)
  pio->PIO_PDR = pin_mask;
}

void pin_enable_pullup(Pio *pio, const unsigned pin_idx,
    const bool enable_pullup)
{
  pin_enable_pio(pio);
  pio->PIO_PUER |= 1 << pin_idx;
}

void pin_set_output(Pio *pio, const unsigned pin_idx,
    const bool output_state)
{
  if (pin_idx > 31)
    return; // adios amigo
  pin_enable_pio(pio);
  const unsigned pin_mask = 1 << pin_idx;
  // set output register
  if (output_state)
    pio->PIO_SODR = pin_mask;
  else
    pio->PIO_CODR = pin_mask;
  // enable GPIO control of pin
  pio->PIO_OER = pin_mask;
  pio->PIO_PER = pin_mask;
}

void pin_set_output_state(Pio *pio, const unsigned pin_idx,
    const bool output_state)
{
  if (pin_idx > 31)
    return; // adios amigo
  const unsigned pin_mask = 1 << pin_idx;
  if (output_state)
    pio->PIO_SODR = pin_mask;
  else
    pio->PIO_CODR = pin_mask;
}

#if 0
void pin_set_output_type(GPIO_TypeDef *gpio, 
                         const uint8_t pin_idx,
                         const uint8_t output_type)
{
  pin_enable_gpio(gpio);
  if (output_type == PIN_OUTPUT_TYPE_OPEN_DRAIN)
  {
    printf("setting pin %d to open-drain\r\n", pin_idx);
    gpio->OTYPER |= (1 << pin_idx);
  }
  else
  {
    printf("setting pin %d to push-pull\r\n", pin_idx);
    gpio->OTYPER &= ~(1 << pin_idx);
  }
}

void pin_set_output(GPIO_TypeDef *gpio, 
                    const uint8_t pin_idx, 
                    const uint8_t initial_state)
{
  if (pin_idx > 15)
    return; // adios amigo
  pin_enable_gpio(gpio);
  pin_set_output_state(gpio, pin_idx, initial_state);
  gpio->MODER &= ~(3 << (pin_idx * 2));
  gpio->MODER |= 1 << (pin_idx * 2);
}

void pin_set_analog(GPIO_TypeDef *gpio, const uint8_t pin_idx)
{
  if (pin_idx > 15)
    return; // adios amigo
  pin_enable_gpio(gpio);
  gpio->MODER |= 3 << (pin_idx * 2);
}

void pin_set_output_state(GPIO_TypeDef *gpio, 
                          const uint8_t pin_idx, 
                          const uint8_t state)
{
  if (state)
    gpio->BSRR = 1 << pin_idx;
  else
    gpio->BSRR = (1 << pin_idx) << 16;
}

void pin_set_output_speed(GPIO_TypeDef *gpio,
                          const uint_fast8_t pin_idx,
                          const uint_fast8_t speed)
{
  pin_enable_gpio(gpio);
  if (pin_idx > 15)
    return;
  if (speed > 3)
    return;
  gpio->OSPEEDR &= ~(0x3 << (pin_idx * 2)); // wipe out the old setting
  gpio->OSPEEDR |= speed << (pin_idx * 2);  // stuff in the new one
}

void pin_toggle_state(GPIO_TypeDef *gpio, const uint8_t pin_idx)
{
  gpio->ODR ^= (1 << pin_idx);
}

void pin_set_input(GPIO_TypeDef *gpio, const uint8_t pin_idx, 
                   const bool enable_pullup)
{
  if (pin_idx > 16)
    return; // garbage
  pin_enable_gpio(gpio);
  gpio->MODER &= ~(3 << (pin_idx * 2)); // mode 0 = input
  if (enable_pullup)
  {
    gpio->PUPDR |= 1 << (pin_idx * 2);
  }
}
#endif
