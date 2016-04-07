#ifndef FREERTPS_METAL_H
#define FREERTPS_METAL_H

// definitions of metal:
//   1) a most excellent style of music
//   2) running without an operating system. BUCKLE YOUR SEAT BELTS KIDS

static inline void freertps_metal_enable_irq(void)
{
  __asm volatile ("cpsie i" : : : "memory");
}

static inline void freertps_metal_disable_irq(void)
{
  __asm volatile ("cpsid i" : : : "memory");
}

#endif
