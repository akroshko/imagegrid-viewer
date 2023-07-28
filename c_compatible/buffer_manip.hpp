#ifndef BUFFER_MANIP_HPP
#define BuFFER_MANIP_HPP

#include "../debug.hpp"
#include "../error.hpp"
#include "../types.hpp"
#include <tiffio.h>

/**
 * Copy and reduce size of RGB buffer from tiff files.
 *
 * @param source_buffer the source buffer from libtiff
 *
 * @param w the width of the libtiff image pixels
 *
 * @param h the height of the libtiff image pixels
 *
 * @param dest_buffer the destination buffer
 *
 * @param w_reduced the width of the destination buffer in pixels
 *
 * @param h_reduced the height of the destination buffer in pixels
 *
 * @param zoom_index the factor to the reduce the image by, must be a power of 2
 */
void buffer_copy_reduce_tiff (uint32* source_buffer, uint32 w, uint32 h,
                              unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT_T zoom_index);

/**
 * Copy and reduce size of generic RGB buffer.
 *
 * @param source_buffer the source buffer
 *
 * @param w the width of the libtiff image pixels
 *
 * @param h the height of the libtiff image pixels
 *
 * @param dest_buffer the destination buffer
 *
 * @param w_reduced the width of the destination buffer in pixels
 *
 * @param h_reduced the height of the destination buffer in pixels
 *
 * @param zoom_index the factor to the reduce the image by, must be a power of 2
 */
void buffer_copy_reduce_generic (unsigned char* source_buffer, size_t w, size_t h,
                                 unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                                 INT_T zoom_index);
#endif