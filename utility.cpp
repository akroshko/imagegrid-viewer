/**
 * Some utility functions.
 */
#include "common.hpp"
#include "utility.hpp"
// C++ headers
#include <string>
#include <thread>
// C headers
#include <cmath>

INT64 reduce_and_pad(INT64 x, INT64 reduction_factor) {
  auto padded_x=pad(x, reduction_factor);
  // TODO: figure out where these zeros are coming from
  //       but generally I want no change
  INT64 reduced_x;
  if (reduction_factor == 0) {
    reduced_x=padded_x;
  } else {
    reduced_x=padded_x/reduction_factor;
  }
  return reduced_x;
}

INT64 pad (INT64 x, INT64 pad_size) {
  if (pad_size == 0 || x % pad_size == 0) {
    return x;
  } else {
    auto pad_x=x + (pad_size - (x % pad_size));
    return pad_x;
  }
}

void sleep_thread () {
  // do a minisleep after each file is unloaded to make sure other
  // things can happen
  // std::this_thread::yield();
  // std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

bool check_valid_filename (const std::string& filename) {
  return filename != "";
}

INT64 shift_right_signed (INT64 num, INT64 shift) {
  if (shift == 0) {
    return num;
  } else if (shift < 0) {
    return num << -shift;
  } else {
    return num >> shift;
  }
}

INT64 shift_left_signed (INT64 num, INT64 shift) {
  if (shift == 0) {
    return num;
  } else if (shift < 0) {
    return num >> -shift;
  } else {
    return num << shift;
  }
}

FLOAT64 ceil_minus_one (FLOAT64 num) {
  auto ceil_test=ceil(num);
  if (ceil_test != num) {
    return ceil_test-1;
  } else {
    return num;
  }
}
