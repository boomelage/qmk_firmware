#pragma once

#define HAL_USE_I2C TRUE
#define HAL_USE_SPI TRUE
#define SPI_USE_WAIT TRUE
#define SPI_SELECT_MODE SPI_SELECT_MODE_PAD
#define PAL_USE_CALLBACKS TRUE

// Screen module talks over UART3 (SD3). The wireless makefile normally pulls in
// the serial driver; this variant drops that makefile, so enable it here.
#define HAL_USE_SERIAL TRUE

#include_next <halconf.h>
