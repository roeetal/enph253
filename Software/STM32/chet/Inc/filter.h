#ifndef FILTER_H
#include"stm32f1xx_hal.h"
#define FILTER_H

#define PI 3.141592653589793

double goertzel(uint32_t *x, uint32_t sample_rate, uint16_t freq, uint16_t window_size, uint8_t offset);

#endif
