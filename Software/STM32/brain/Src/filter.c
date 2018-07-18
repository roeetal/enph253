#include "stm32f4xx.h"
#include "math.h"
#include "filter.h"

double goertzel(uint32_t *x, uint32_t sample_rate, uint16_t freq, uint16_t window_size, uint8_t offset)
{
  // Set up initial parameters
  double f_step = sample_rate / (double)window_size;
  double f_step_normalized = 1.0 / (double)window_size;
  double k = freq / f_step;

  // number of frequencies is the same size as the number of
  // upper and lower bin limits; loop through bin limits (i.e.
  // k_start and k_end pairs, and evaluate goertzel from there)
  double normalizedfreq, w_real;

  normalizedfreq = k * f_step_normalized;
  w_real = 2.0 * cos(2.0 * PI * normalizedfreq);
  double d1 = 0, d2 = 0, y = 0, tot_power = 1;
  uint16_t n = offset;
  while (n < window_size)
  {
    y = x[n] + w_real * d1 - d2;
    d2 = d1;
    d1 = y;
    tot_power += x[n] * x[n];
    n += 3;
  }

  return (d2 * d2 + d1 * d1 - w_real * d1 * d2) / tot_power;
}