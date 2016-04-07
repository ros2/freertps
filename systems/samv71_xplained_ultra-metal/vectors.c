/*  Software License Agreement (Apache License)
 *
 *  Copyright 2015 Open Source Robotics Foundation
 *  Author: Morgan Quigley
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdint.h>
#include "metal/stack.h"

// reset_vector has to exist, but we'll declare the rest as weak vectors 
// and map them to the "unmapped_vector()" trap.
extern void reset_vector(void); 

void unmapped_vector(void)
{
  while (1); // IT'S A TRAP!!! spin here to allow JTAG to capture it
}

// declare weak symbols for all the interrupts 
// (so that applications can override them without having to declare their
// own interrupt tables)

#define WEAK_VECTOR __attribute__((weak, alias("unmapped_vector")))
void nmi_vector(void)                WEAK_VECTOR;
void hard_fault_vector(void)         WEAK_VECTOR;
void memory_management_vector(void)  WEAK_VECTOR;
void bus_fault_vector(void)          WEAK_VECTOR;
void usage_fault_vector(void)        WEAK_VECTOR;
void svcall_vector(void)             WEAK_VECTOR;
void pendsv_vector(void)             WEAK_VECTOR;
void systick_vector(void)            WEAK_VECTOR;
void supc_vector(void)               WEAK_VECTOR;
void rstc_vector(void)               WEAK_VECTOR;
void rtc_vector(void)                WEAK_VECTOR;
void rtt_vector(void)                WEAK_VECTOR;
void wdt_vector(void)                WEAK_VECTOR;
void pmc_vector(void)                WEAK_VECTOR;
void eefc0_vector(void)              WEAK_VECTOR;
void eefc1_vector(void)              WEAK_VECTOR;
void uart0_vector(void)              WEAK_VECTOR;
void uart1_vector(void)              WEAK_VECTOR;
void uart2_vector(void)              WEAK_VECTOR;
void uart3_vector(void)              WEAK_VECTOR;
void uart4_vector(void)              WEAK_VECTOR;
void smc_vector(void)                WEAK_VECTOR;
void pioa_vector(void)               WEAK_VECTOR;
void piob_vector(void)               WEAK_VECTOR;
void pioc_vector(void)               WEAK_VECTOR;
void piod_vector(void)               WEAK_VECTOR;
void pioe_vector(void)               WEAK_VECTOR;
void usart0_vector(void)             WEAK_VECTOR;
void usart1_vector(void)             WEAK_VECTOR;
void usart2_vector(void)             WEAK_VECTOR;
void hsmci_vector(void)              WEAK_VECTOR;
void twi0_vector(void)               WEAK_VECTOR;
void twi1_vector(void)               WEAK_VECTOR;
void twihs2_vector(void)             WEAK_VECTOR;
void spi0_vector(void)               WEAK_VECTOR;
void spi1_vector(void)               WEAK_VECTOR;
void qspi_vector(void)               WEAK_VECTOR;
void ssc_vector(void)                WEAK_VECTOR;
void tc0_vector(void)                WEAK_VECTOR;
void tc1_vector(void)                WEAK_VECTOR;
void tc2_vector(void)                WEAK_VECTOR;
void tc3_vector(void)                WEAK_VECTOR;
void tc4_vector(void)                WEAK_VECTOR;
void tc5_vector(void)                WEAK_VECTOR;
void tc6_vector(void)                WEAK_VECTOR;
void tc7_vector(void)                WEAK_VECTOR;
void tc8_vector(void)                WEAK_VECTOR;
void tc9_vector(void)                WEAK_VECTOR;
void tc10_vector(void)               WEAK_VECTOR;
void tc11_vector(void)               WEAK_VECTOR;
void afec0_vector(void)              WEAK_VECTOR;
void afec1_vector(void)              WEAK_VECTOR;
void dac_vector(void)                WEAK_VECTOR;
void pwm0_vector(void)               WEAK_VECTOR;
void pwm1_vector(void)               WEAK_VECTOR;
void crccu_vector(void)              WEAK_VECTOR;
void acc_vector(void)                WEAK_VECTOR;
void usbhs_vector(void)              WEAK_VECTOR;
void icm_vector(void)                WEAK_VECTOR;
void mcan0_vector(void)              WEAK_VECTOR;
void mcan0_line_vector(void)         WEAK_VECTOR;
void mcan1_vector(void)              WEAK_VECTOR;
void mcan1_line_vector(void)         WEAK_VECTOR;
void gmac_vector(void)               WEAK_VECTOR;
void mlb_vector(void)                WEAK_VECTOR;
void aes_vector(void)                WEAK_VECTOR;
void trng_vector(void)               WEAK_VECTOR;
void xdmac_vector(void)              WEAK_VECTOR;
void isi_vector(void)                WEAK_VECTOR;
void sdramc_vector(void)             WEAK_VECTOR;
void rswdt_vector(void)              WEAK_VECTOR;


// this is only used to ensure that this file is linked into the final
// executable; we set the actual reset_vector below in g_vectors[]
void placeholder_reset_vector(void) { }

typedef void (*vector_func_t)(void);

__attribute__((used,section(".vectors")))
vector_func_t g_vectors[] = {
    (vector_func_t)(&g_stack[STACK_SIZE-8]),
    reset_vector,
    nmi_vector,
    hard_fault_vector,
    memory_management_vector, 
    bus_fault_vector, 
    usage_fault_vector,
    0, 0, 0, 0,               // 7,8,9,10  reserved
    svcall_vector,            // 11 
    0, 0,                     // 12, 13, reserved
    pendsv_vector,            // 14
    systick_vector,           // 15
    ///////////////////////////////////////////////////////////////
    supc_vector,   // 0 supply controller 
    rstc_vector,   // 1 reset controller 
    rtc_vector,    // 2 real time clock 
    rtt_vector,    // 3 real time timer 
    wdt_vector,    // 4 watchdog timer 
    pmc_vector,    // 5 power management controller
    eefc0_vector,  // 6 enhanced embedded flash controller
    uart0_vector,  // 7
    uart1_vector,  // 8
    0,             // 9
    pioa_vector,   // 10
    piob_vector,   // 11
    pioc_vector,   // 12
    usart0_vector, // 13 
    usart1_vector, // 14
    usart2_vector, // 15
    piod_vector,   // 16
    pioe_vector,   // 17
    hsmci_vector,  // 18 multimedia card interface
    twi0_vector,   // 19 i2c0
    twi1_vector,   // 20 i2c1
    spi0_vector,   // 21 
    ssc_vector,    // 22 synchronous serial controller
    tc0_vector,    // 23 timer/counter 0
    tc1_vector,    // 24 
    tc2_vector,    // 25
    tc3_vector,    // 26
    tc4_vector,    // 27
    tc5_vector,    // 28
    afec0_vector,  // 29 analog front end (fancier than just an ADC...)
    dac_vector,    // 30
    pwm0_vector,   // 31
    icm_vector,    // 32 integrity check monitor
    acc_vector,    // 33 analog comparator
    usbhs_vector,  // 34 high-speed usb controller
    mcan0_vector,  // 35 canbus 0
    mcan0_line_vector, // 36 canbus 0
    mcan1_vector,      // 37 canbus 1
    mcan1_line_vector, // 38 canbus 1
    gmac_vector,      // 39 ethernet
    afec1_vector,     // 40 analog front end 1
    twihs2_vector,    // 41 i2c2
    spi1_vector,      // 42
    qspi_vector,      // 43 quad-spi interface
    uart2_vector,     // 44
    uart3_vector,     // 45
    uart4_vector,     // 46
    tc6_vector,       // 47
    tc7_vector,       // 48
    tc8_vector,       // 49
    tc9_vector,       // 50
    tc10_vector,      // 51
    tc11_vector,      // 52
    mlb_vector,       // 53 media stuff (automotive)
    0, 0,             // 54, 55 reserved
    aes_vector,       // 56 crypto
    trng_vector,      // 57 random generator
    xdmac_vector,     // 58 not sure
    isi_vector,       // 59 image sensor interface
    pwm1_vector,      // 60
    0,                // 61 reserved
    sdramc_vector,    // 62 memory interface controller
    rswdt_vector,     // 63 reinforced secure watchdog timer?
};
