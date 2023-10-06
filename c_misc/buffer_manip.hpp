#ifndef BUFFER_MANIP_HPP
#define BUFFER_MANIP_HPP

#include "../common.hpp"
#include <tiffio.h>

/**
 * Copy and reduce size of RGB buffer from tiff files.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param w The width of the libtiff image pixels.
 * @param h The height of the libtiff image pixels.
 * @param dest_buffer The destination buffer.
 * @param w_reduced The width of the destination buffer in pixels.
 * @param h_reduced The height of the destination buffer in pixels.
 * @param zoom_index The factor to the reduce the image by, must be a
 *                   power of 2.
 */
void buffer_copy_reduce_tiff (uint32_t* source_buffer, uint32_t w, uint32_t h,
                              unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT_T zoom_index);

/**
 * Copy and reduce size of generic RGB buffer.
 *
 * @param source_buffer The source buffer.
 * @param w The width of the image pixels.
 * @param h The height of the image pixels.
 * @param x_origin An x offset for the origin.
 * @param y_origin A y offset for the origin.
 * @param dest_buffer The destination buffer.
 * @param w_reduced The width of the destination buffer in pixels.
 * @param h_reduced The height of the destination buffer in pixels.
 * @param zoom_index The factor to the reduce the image by, must be a
 *                   power of 2.
 */
void buffer_copy_reduce_generic (unsigned char* source_buffer, size_t w, size_t h,
                                     INT_T x_origin, INT_T y_origin,
                                     unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                                     INT_T zoom_index);

/**
 * Copy and expand size of generic RGB buffer.
 *
 * @param source_buffer The source buffer.
 * @param w The width of the image pixels.
 * @param h The height of the image pixels.
 * @param x_origin An x offset for the origin.
 * @param y_origin A y offset for the origin.
 * @param dest_buffer The destination buffer.
 * @param w_expanded The width of the destination buffer in pixels.
 * @param h_expanded The height of the destination buffer in pixels.
 * @param zoom_index The factor to the reduce the image by, must be a
 *                   power of 2.
 */
void buffer_copy_expand_generic (unsigned char* source_buffer, size_t w, size_t h,
                                 INT_T x_origin, INT_T y_origin,
                                 unsigned char* dest_buffer, size_t w_expanded, size_t h_expanded,
                                 INT_T zoom_index);

#endif