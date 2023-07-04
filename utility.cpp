/**
 * Some utility functions.
 */
#include "types.hpp"
#include "utility.hpp"
// C++ headers
#include <cmath>

INT_T find_zoom_index(FLOAT_T zoom) {
  return floor(log2(1.0/zoom));
}

// these functions are specific to how coordinates that are integers are
FLOAT_T next_smallest(FLOAT_T x) {
  return floor(x) - 1.0;
}

FLOAT_T next_largest(FLOAT_T x) {
  if (x == ceil(x)) {
    return x + 1.0;
  } else {
    return ceil(x);
  }
}

FLOAT_T half(FLOAT_T x) {
  return x;
}

// round down to the nearest power of 2
INT_T round_down_power_of_2(FLOAT_T x) {
  auto log_2=log2(x);
  auto log_2_floor=floor(log_2);
  return (INT_T)pow(2.0,log_2_floor);

}

INT_T reduce_and_pad(INT_T x, INT_T reduction_factor) {
  auto padded_x=pad(x, reduction_factor);
  auto reduced_x=padded_x/reduction_factor;
  return reduced_x;
}

INT_T pad (INT_T x, INT_T pad_size) {
  if (x % pad_size == 0) {
    return x;
  } else {
    auto pad_x=x + (pad_size - (x % pad_size));
    return pad_x;
  }
}
