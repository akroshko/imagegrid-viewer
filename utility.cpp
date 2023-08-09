/**
 * Some utility functions.
 */
#include "defaults.hpp"
#include "types.hpp"
#include "utility.hpp"
// C++ headers
#include <cmath>
#include <thread>

/**
 * Pad a number and then reduce by a factor.
 *
 * @param x the number to pad and reduce
 *
 * @param reduction_factor The factor to reduce the number by.
 *
 */
INT_T reduce_and_pad(INT_T x, INT_T reduction_factor) {
  auto padded_x=pad(x, reduction_factor);
  // TODO: figure out where these zeros are coming from
  //       but generally I want no change
  INT_T reduced_x;
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
 *
 * @param pad_size The amount of padding to add to make it a multiple
 *                 of this.
 */
INT_T pad (INT_T x, INT_T pad_size) {
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
  std::this_thread::sleep_for(THREAD_SLEEP);
}
