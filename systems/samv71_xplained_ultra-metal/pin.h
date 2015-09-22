#ifndef PIN_H
#define PIN_H

#include <stdbool.h>
#include <stdint.h>
#include "component/component_pio.h"

// ported from the STM32 stuff... not yet all implemented

void pin_set_mux(Pio *pio, const unsigned pin_idx,
    const unsigned function_idx);

void pin_enable_pullup(Pio *pio, const unsigned pin_idx,
    const bool enable_pullup);

void pin_set_output(Pio *pio, const unsigned pin_idx,
    const bool output_state);

void pin_set_output_state(Pio *pio, const unsigned pin_idx,
    const bool output_state);


#define PERIPH_A 0
#define PERIPH_B 1
#define PERIPH_C 2
#define PERIPH_D 3

#if 0
#define PIN_OUTPUT_TYPE_PUSH_PULL  0
#define PIN_OUTPUT_TYPE_OPEN_DRAIN 1

void pin_set_output_type(GPIO_TypeDef *gpio, 
                         const uint8_t pin_idx,
                         const uint8_t output_type);

void pin_set_analog(GPIO_TypeDef *gpio, const uint8_t pin_idx);

void pin_set_output(GPIO_TypeDef *gpio, 
                    const uint8_t pin_idx,
                    const uint8_t initial_state);
void pin_set_output_speed(GPIO_TypeDef *gpio,
                          const uint_fast8_t pin_idx,
                          const uint_fast8_t speed);

void pin_set_output_state(GPIO_TypeDef *gpio,
                          const uint8_t pin_idx,
                          const uint8_t state);

void pin_toggle_state(GPIO_TypeDef *gpio, const uint8_t pin_idx);

void pin_set_input(GPIO_TypeDef *gpio, const uint8_t pin_idx, 
                   const bool enable_pullup);

static inline void pin_set_output_high(GPIO_TypeDef *gpio, 
                                const uint8_t pin_idx)
{
  gpio->BSRR = 1 << pin_idx;
}

static inline void pin_set_output_low(GPIO_TypeDef *gpio, 
                               const uint8_t pin_idx)
{
  gpio->BSRR = (1 << pin_idx) << 16;
}

static inline bool pin_get_state(GPIO_TypeDef *gpio, const uint_fast8_t pin_idx)
{
  return (gpio->IDR & (1 << pin_idx)) ? true : false;
}
#endif

#endif
