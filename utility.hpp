/**
 * Header for utility functions.
 */
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "common.hpp"

INT_T round_down_positive_power_of_2(FLOAT_T x);

INT_T reduce_and_pad(INT_T x, INT_T reduction_factor);

INT_T pad (INT_T x, INT_T pad_size);

void sleep_thread();

#endif
