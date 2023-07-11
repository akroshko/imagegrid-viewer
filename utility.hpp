/**
 * Header for utility functions.
 */
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "types.hpp"

INT_T find_zoom_index(FLOAT_T zoom);

FLOAT_T next_smallest(FLOAT_T x);

FLOAT_T next_largest(FLOAT_T x);

FLOAT_T half(FLOAT_T x);

INT_T round_down_power_of_2(FLOAT_T x);

INT_T reduce_and_pad(INT_T x, INT_T reduction_factor);

INT_T pad (INT_T x, INT_T pad_size);

void sleep_thread();

#define DELETE_IF_NOT_NULLPTR(A) if(A != nullptr) {delete A; A=nullptr;}
#define DELETE_ARRAY_IF_NOT_NULLPTR(A) if(A != nullptr) {delete[] A; A=nullptr;}
#endif
