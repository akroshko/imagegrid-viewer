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
 * @param x the number to pad and reduce
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
 * Sleep and yield current thread.
 */
void sleep_thread();

/**
 * Check whether a filename is valid.
 *
 * Just checks whether name is empty string for now, but could
 * potentially become more complex
 *
 * @param filename The filename to check.
 * @return Whether the filename is valid.
 */
bool check_valid_filename (std::string filename);

#endif
