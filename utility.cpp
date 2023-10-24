/**
 * Some utility functions.
 */
#include "common.hpp"
#include "utility.hpp"
// C++ headers
#include <thread>
// C headers
// #include <cmath>

/**
 * Pad a number and then reduce by a factor.
 *
 * @param x the number to pad and reduce
 * @param reduction_factor The factor to reduce the number by.
 * @return The reduced and padded value.
 */
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

/**
 * Pad a number.
 *
 * @param x The number to pad.
 * @param pad_size The amount of padding to add to make it a multiple
 *                 of this.
 * @return The padded value.
 */
INT64 pad (INT64 x, INT64 pad_size) {
  if (pad_size == 0 || x % pad_size == 0) {
    return x;
  } else {
    auto pad_x=x + (pad_size - (x % pad_size));
    return pad_x;
  }
}

/**
 * Sleep and yield current thread.
 */
void sleep_thread () {
  // do a minisleep after each file is unloaded to make sure other
  // things can happen
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
