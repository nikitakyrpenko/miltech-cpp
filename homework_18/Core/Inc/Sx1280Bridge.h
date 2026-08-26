#pragma once

#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void Sx1280_Init(SPI_HandleTypeDef* hspi,
                 GPIO_TypeDef* nssPort,
                 GPIO_TypeDef* busy_port,
                 GPIO_TypeDef* nreset_port,
                 uint16_t nssPin,
                 uint16_t busy_pin,
                 uint16_t nreset_pin);
void Sx1280_Loop(void);

#ifdef __cplusplus
}
#endif
