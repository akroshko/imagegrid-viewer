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
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_tiff (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                              PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                              INT64 zoom_out_shift,
                              INT64* row_buffer);

/**
 * Copy an RGBA buffer from tiff files.  A "safe" version that should
 * work with any zoom_index and on any 64-bit platform.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 */
void buffer_copy_noreduce_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                     PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h);

/**
 * Copy and reduce size by 2 of an RGBA buffer from tiff files.  A
 * "safe" version that should work with any zoom_index and on any
 * 64-bit platform.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param row_buffer A working buffer of size at least (source_copy_w >> 2)*3).
 */
void buffer_copy_reduce_2_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                     PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                     INT64* row_buffer);

/**
 * Copy and reduce size by maximum factor of 8 an RGBA buffer from
 * tiff files.  A "safe" version that should work with any zoom_index
 * and on any 64-bit platform.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param zoom_out_shift The factor to reduce the image by as a bit
 *                       shift, maximum 3, i.e., log2(8).
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_max_8_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                         PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                         INT64 zoom_out_shift,
                                         INT64* row_buffer);

/**
 * Copy and reduce size of an RGBA buffer from tiff files.  A "safe"
 * version that should work with any zoom_index and on any 64-bit
 * platform.
 *
 * @param source_buffer The source buffer from libtiff.
 * @param source_w The width of the libtiff source pixels.
 * @param source_h The height of the libtiff source pixels.
 * @param dest_buffer The destination buffer.
 * @param dest_w The width of the destination buffer in pixels.
 * @param dest_h The height of the destination buffer in pixels.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                   PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                   INT64 zoom_out_shift,
                                   INT64* row_buffer);

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
 * @param dest_w_limit The max width to use in the destination buffer.
 * @param dest_h_limit The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_limit, INT64 dest_h_limit,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out_shift,
                                 INT64* row_buffer);

/**
 * Copy without reducing size of a generic RGBA buffer. A "safe"
 * version that should work on any 64-bit platform.
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
 * @param dest_w_limit The max width to use in the destination buffer.
 * @param dest_h_limit The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 */
void buffer_copy_noreduce_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                        INT64 source_start_x, INT64 source_start_y,
                                        INT64 source_copy_w, INT64 source_copy_h,
                                        PIXEL_RGBA* dest_buffer,
                                        INT64 dest_w, INT64 dest_h,
                                        INT64 dest_w_limit, INT64 dest_h_limit,
                                        INT64 dest_start_x, INT64 dest_start_y);

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
 * @param dest_w_limit The max width to use in the destination buffer.
 * @param dest_h_limit The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_out_shift The factor to expand the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_limit, INT64 dest_h_limit,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_out_shift,
                                      INT64* row_buffer);

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
 * @param dest_w_limit The max width to use in the destination buffer.
 * @param dest_h_limit The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_limit, INT64 dest_h_limit,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_in_shift);

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
 * @param dest_w_limit The max width to use in the destination buffer.
 * @param dest_h_limit The max height to use in the destination buffer.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_limit, INT64 dest_h_limit,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in_shift);

#endif
