/**
 * Header for utility functions.
 */
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "common.hpp"

INT64 round_down_positive_power_of_2(FLOAT64 x);

INT64 reduce_and_pad(INT64 x, INT64 reduction_factor);

INT64 pad (INT64 x, INT64 pad_size);

void sleep_thread();

#endif
