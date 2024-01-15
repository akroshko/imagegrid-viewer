#ifndef BUFFER_MANIP_HPP
#define BUFFER_MANIP_HPP

#include "../common.hpp"
#include "../datatypes/coordinates.hpp"
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
 * @param source_size The size of the source buffer from libtiff.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_tiff (const uint32_t* const source_buffer,
                              const BufferPixelSize& source_size,
                              PIXEL_RGBA* dest_buffer,
                              const BufferPixelSize& dest_size,
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
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 * @param zoom_out_shift The factor to reduce the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void buffer_copy_reduce_standard (const PIXEL_RGBA* const source_buffer,
                                  const BufferPixelSize& source_size,
                                  const BufferPixelCoordinate& source_start,
                                  const BufferPixelSize& source_copy_size,
                                  PIXEL_RGBA* dest_buffer,
                                  const BufferPixelSize& dest_size,
                                  const BufferPixelSize& dest_size_visible,
                                  const BufferPixelCoordinate& dest_start,
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
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic (const PIXEL_RGBA* const source_buffer,
                                 const BufferPixelSize& source_size,
                                 const BufferPixelCoordinate& source_start,
                                 const BufferPixelSize& source_copy_size,
                                 PIXEL_RGBA* dest_buffer,
                                 const BufferPixelSize& dest_size,
                                 const BufferPixelSize& dest_size_visible,
                                 const BufferPixelCoordinate& dest_start,
                                 INT64 zoom_in_shift);

/**
 * Copy and expand size of a generic RGBA buffer. A "safe" version
 * that should work with any zoom_out_shift and on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_size
 * @param source_start
 * @param source_copy_size
 * @param dest_buffer The destination buffer.
 * @param dest_size
 * @param dest_size_visible
 * @param dest_start
 * @param zoom_in_shift The factor to expand the image by as a bit shift.
 */
void buffer_copy_expand_generic_safe (const PIXEL_RGBA* const source_buffer,
                                      const BufferPixelSize& source_size,
                                      const BufferPixelCoordinate& source_start,
                                      const BufferPixelSize& source_copy_size,
                                      PIXEL_RGBA* dest_buffer,
                                      const BufferPixelSize& dest_size,
                                      const BufferPixelSize& dest_size_visible,
                                      const BufferPixelCoordinate& dest_start,
                                      INT64 zoom_in_shift);
#endif
