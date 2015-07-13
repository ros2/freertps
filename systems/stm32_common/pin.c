#include "pin.h"
#include <stdio.h>

void pin_enable_gpio(GPIO_TypeDef *gpio)
{
  if (gpio == GPIOA)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  else if (gpio == GPIOB)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  else if (gpio == GPIOC)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  else if (gpio == GPIOD)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  else if (gpio == GPIOE)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
  else if (gpio == GPIOF)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
  else if (gpio == GPIOG)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
  else if (gpio == GPIOH)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
  else if (gpio == GPIOI)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
}

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

void pin_set_alternate_function(GPIO_TypeDef *gpio,
                                const uint8_t pin_idx,
                                const uint8_t function_idx)
{
  if (pin_idx > 15 || function_idx > 15)
    return; // adios amigo
  pin_enable_gpio(gpio);
  volatile uint32_t *af_reg = (pin_idx < 8) ? &gpio->AFR[0] : &gpio->AFR[1];
  const uint8_t reg_ofs = (pin_idx < 8) ? (pin_idx * 4) : ((pin_idx-8) * 4);
  *af_reg &= ~(0xf << reg_ofs); // zero out whatever was there before
  *af_reg |= function_idx << reg_ofs; // set the alternate function register
  gpio->MODER &= ~(3 << (pin_idx * 2)); // zero out whatever was there before
  gpio->MODER |= 2 << (pin_idx * 2); // put the GPIO in alternate-function mode
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

