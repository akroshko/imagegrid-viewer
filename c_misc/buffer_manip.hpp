#ifndef BUFFER_MANIP_HPP
#define BUFFER_MANIP_HPP

#include "../common.hpp"
#include <tiffio.h>

/**
 * Copy and reduce size of an RGBA buffer from tiff files.
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
                              PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT64 zoom_index);

/**
 * Copy and reduce size of a generic RGBA buffer.
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
void buffer_copy_reduce_generic (PIXEL_RGBA* source_buffer, size_t w, size_t h,
                                 INT64 x_origin, INT64 y_origin,
                                 PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                                 INT64 zoom_index);

/**
 * Copy and expand size of a generic RGBA buffer.
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
void buffer_copy_expand_generic (PIXEL_RGBA* source_buffer, size_t w, size_t h,
                                 INT64 x_origin, INT64 y_origin,
                                 PIXEL_RGBA* dest_buffer, size_t w_expanded, size_t h_expanded,
                                 INT64 zoom_index);

#endif
