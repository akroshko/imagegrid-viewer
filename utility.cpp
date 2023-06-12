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
