#include "utility.hpp"
// C headers
#include <math.h>

int find_zoom_index(float zoom) {
  return floor(log2(1.0/zoom));
}

// these functions are specific to how coordinates that are integers are
float next_smallest(float x) {
  return floor(x) - 1.0;
}

float next_largest(float x) {
  if (x == ceil(x)) {
    return x + 1.0;
  } else {
    return ceil(x);
  }
}
