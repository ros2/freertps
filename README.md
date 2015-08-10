# freertps
a free, portable, minimalist RTPS implementation

extremely high awesome factor

## WARNING WARNING WARNING
This code is extremely experimental. It is an abomination. Approximately everything is expected to change rapidly as features are added and cleaned up.
## WARNING WARNING WARNING

### Prerequisites

There are many ways to build this code. Most development is currently happening on Ubuntu 14.04.2 workstations, with the following toolchains:
 * arm-none-eab-gcc 4.9.3 installed from PPA at https://launchpad.net/~terry.guo/+archive/ubuntu/gcc-arm-embedded
 * cmake 2.8.12.2
 * OpenOCD 0.10.0-dev built from latest source tree and modified according to a few patches on the OpenOCD website for STM32F7 support

### Compiling
 
To build it, just type "make" and it will spin CMake up a few times to build freertps for a few different systems.

Then, to try it out on the STM32F7-Discovery board, you can do this (providing that you have the latest source (not release) of OpenOCD, and have cherry-picked a few patches floating around the mailing list to add support for STM32F7:

```
make program-listener-stm32f7_disco-metal
```

that will start the board with an RTPS listener that will just `printf` anything that comes to the `chatter` topic to USART6 @ 1 Mbps, which you would then need to bring back to your workstation via a 3.3v UART-to-USB cable (e.g., from FTDI).

You can also flash the board with the 'talker' program:

```
make program-talker-stm32f7_disco-metal
```

That will transmit "hello, world" strings on the `chatter` topic at 2 Hz.

To test fast-moving stuff, you can do this:

```
make program-talker_stm32_timer-stm32f7_disco-metal
```

That will use a hardware timer to send "hello, world" messages at exactly 1 kHz.

You can substitute "stm32f7_disco" for "stm32f4_disco" and it will flash the STM32F4-Discovery board with those same programs; they will run on either platform (hopefully).
