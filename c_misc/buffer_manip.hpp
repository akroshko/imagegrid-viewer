#ifndef BUFFER_MANIP_HPP
#define BUFFER_MANIP_HPP

#include "../common.hpp"
// C headers
#include <cstddef>
#include <cstdint>

#define TIFF_SOURCE_TYPE const uint32_t* const
#define TIFF_NOREDUCE_FUNCNAME buffer_copy_noreduce_tiff_safe
#define TIFF_REDUCE2_FUNCNAME buffer_copy_reduce_2_tiff_safe
#define TIFF_REDUCE_MAX_8_FUNCNAME buffer_copy_reduce_max_8_tiff_safe
#define TIFF_REDUCE_ALL_FUNCNAME buffer_copy_reduce_tiff_safe

#define GENERIC_SOURCE_TYPE const PIXEL_RGBA* const
#define GENERIC_NOREDUCE_FUNCNAME buffer_copy_noreduce_standard_safe
#define GENERIC_REDUCE2_FUNCNAME buffer_copy_reduce_2_standard_safe
#define GENERIC_REDUCE_MAX_8_FUNCNAME buffer_copy_reduce_max_8_standard_safe
#define GENERIC_REDUCE_ALL_FUNCNAME buffer_copy_reduce_standard_safe

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
                              INT64* const row_buffer);

#define SOURCE_TYPE TIFF_SOURCE_TYPE
#define NOREDUCE_FUNCNAME TIFF_NOREDUCE_FUNCNAME
#define REDUCE2_FUNCNAME TIFF_REDUCE2_FUNCNAME
#define REDUCE_MAX_8_FUNCNAME TIFF_REDUCE_MAX_8_FUNCNAME
#define REDUCE_ALL_FUNCNAME TIFF_REDUCE_ALL_FUNCNAME
#include "buffer_manip_generic_header.hpp"
#undef SOURCE_TYPE
#undef NOREDUCE_FUNCNAME
#undef NOREDUCE_SOURCE_TYPE
#undef REDUCE2_FUNCNAME
#undef REDUCE_MAX_8_FUNCNAME
#undef REDUCE_ALL_FUNCNAME

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
 * @param dest_w_visible The max width to use in the destination buffer.
 * @param dest_h_visible The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_standard (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_visible, INT64 dest_h_visible,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out_shift,
                                 INT64* const row_buffer);

#define SOURCE_TYPE GENERIC_SOURCE_TYPE
#define NOREDUCE_FUNCNAME GENERIC_NOREDUCE_FUNCNAME
#define REDUCE2_FUNCNAME GENERIC_REDUCE2_FUNCNAME
#define REDUCE_MAX_8_FUNCNAME GENERIC_REDUCE_MAX_8_FUNCNAME
#define REDUCE_ALL_FUNCNAME GENERIC_REDUCE_ALL_FUNCNAME
#include "buffer_manip_generic_header.hpp"
#undef SOURCE_TYPE
#undef NOREDUCE_FUNCNAME
#undef REDUCE2_FUNCNAME
#undef REDUCE_MAX_8_FUNCNAME
#undef REDUCE_ALL_FUNCNAME

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
 * @param dest_w_visible The max width to use in the destination buffer.
 * @param dest_h_visible The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_visible, INT64 dest_h_visible,
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
 * @param dest_w_visible The max width to use in the destination buffer.
 * @param dest_h_visible The max height to use in the destination buffer.
 * @param source_start_x The x coordinate on the source to start from.
 * @param source_start_y The y coordinate on the source to start from.
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_visible, INT64 dest_h_visible,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in_shift);

#endif
