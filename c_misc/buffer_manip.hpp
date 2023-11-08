#ifndef BUFFER_MANIP_HPP
#define BUFFER_MANIP_HPP

#include "../common.hpp"
// C headers
#include <cstddef>
#include <cstdint>

/**
 * Copy and reduce size of an RGBA buffer from tiff files.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param w_reduced The width of the destination buffer in pixels.
 * @param h_reduced The height of the destination buffer in pixels.
 * @param zoom_out The factor to the reduce the image by, must be a
 *                 power of 2.
 */
void buffer_copy_reduce_tiff (uint32_t* source_buffer, uint32_t source_w, uint32_t source_h,
                              PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT64 zoom_out);

/**
 * Copy and reduce size of an RGBA buffer from tiff files.  A "safe"
 * version that should work with any zoom_index and on any 64-bit
 * platform.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param w_reduced The width of the destination buffer in pixels.
 * @param h_reduced The height of the destination buffer in pixels.
 * @param zoom_out The factor to the reduce the image by, must be a
 *                 power of 2.
 */
void buffer_copy_reduce_tiff_safe (uint32_t* source_buffer, uint32_t source_w, uint32_t source_h,
                                   PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                                   INT64 zoom_out);


/**
 * Copy and reduce size of a generic RGBA buffer.
 *
 * @param source_buffer The source buffer.
 * @param source_w The width of the source pixels.
 * @param source_h The height of the source pixels.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param source_copy_w The width of the source to copy.
 * @param source_copy_h The height of the source to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_out The factor to the reduce the image by, must be a
 *                 power of 2.
 */
void buffer_copy_reduce_generic (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out);
/**
 * Copy and reduce size of a generic RGBA buffer. A "safe" version
 * that should work with any zoom_index and on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_w The width of the source pixels.
 * @param source_h The height of the source pixels.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param source_copy_w The width of the source to copy.
 * @param source_copy_h The height of the source to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_out The factor to the reduce the image by, must be a
 *                 power of 2.
 */
void buffer_copy_reduce_generic_safe (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_out);

/**
 * Copy and expand size of a generic RGBA buffer.
 *
 * @param source_buffer The source buffer.
 * @param source_w The width of the source pixels.
 * @param source_h The height of the source pixels.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param source_copy_w The width of the source to copy.
 * @param source_copy_h The height of the source to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_in The factor to the expand the image by, must be a
 *                power of 2.
 */
void buffer_copy_expand_generic (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_in);

/**
 * Copy and expand size of a generic RGBA buffer. A "safe" version
 * that should work with any zoom_index and on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_w The width of the image pixels.
 * @param source_h The height of the image pixels.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param source_copy_w The width of the source to copy.
 * @param source_copy_h The height of the source to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param zoom_in The factor to the reduce the image by, must be a
 *                power of 2.
 */
void buffer_copy_expand_generic_safe (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in);

#endif
