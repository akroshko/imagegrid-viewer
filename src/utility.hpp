/**
 * Header for utility functions.
 */
#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "common.hpp"
// CPP headers
#include <string>

/**
 * Pad a number and then reduce by a factor.
 *
 * @param x The number to pad and reduce.
 * @param reduction_factor The factor to reduce the number by.
 * @return The reduced and padded value.
 */
INT64 reduce_and_pad(INT64 x, INT64 reduction_factor);

/**
 * Pad a number.
 *
 * @param x The number to pad.
 * @param pad_size The amount of padding to add to make it a multiple
 *                 of this.
 * @return The padded value.
 */
INT64 pad (INT64 x, INT64 pad_size);

/**
 * Check whether a filename is valid.
 *
 * Just checks whether name is empty string for now, but could
 * potentially become more complex
 *
 * @param filename The filename to check.
 * @return Whether the filename is valid.
 */
bool check_valid_filename (const std::string& filename);

/**
 * Bit shift right, expect when shift is negative then shift left.
 *
 * @param num The number to shift.
 * @param shift The amount to shift by.
 * @return The shifted number.
 */
INT64 shift_right_signed (INT64 num, INT64 shift);

/**
 * Bit shift left, expect when shift is negative then shift right.
 *
 * @param num The number to shift.
 * @param shift The amount to shift by.
 * @return The shifted number.
 */
INT64 shift_left_signed (INT64 num, INT64 shift);

/**
 *
 * @param num
 * @return ceil(num)-1 or just the number if whole.
 */
FLOAT64 ceil_minus_one (FLOAT64 num);

#endif
