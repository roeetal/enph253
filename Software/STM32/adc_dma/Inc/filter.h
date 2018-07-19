#ifndef FILTER_H
#define FILTER_H

#define PI 3.141592653589793

double goertzel(uint32_t *x, uint32_t sample_rate, uint16_t freq, uint16_t window_size);

#endif
