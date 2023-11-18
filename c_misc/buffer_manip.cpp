// local headers
#include "../common.hpp"
#include "buffer_manip.hpp"
// C++ headers
#include <memory>
// C headers
#include <cstring>
#include <cstdint>
// C library headers
#include <tiffio.h>

typedef unsigned char (*rgb_extract)(unsigned char);

#define R_MASK 0xFFL
#define G_SHIFT 8L
#define G_MASK 0xFF00L
#define B_SHIFT 16L
#define B_MASK 0xFF0000L
#define R_MASK_COMPACT 0x00000000FFFFL
#define G_SHIFT_COMPACT 16L
#define G_MASK_COMPACT 0x0000FFFF0000L
#define B_SHIFT_COMPACT 32L
#define B_MASK_COMPACT 0xFFFF00000000L

#define DEFAULT_ALPHA 0xFF000000L

void buffer_copy_reduce_tiff (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                              PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                              INT64 zoom_out_shift,
                              INT64* const row_buffer) {
  if (zoom_out_shift == 0) {
    buffer_copy_noreduce_tiff_safe(source_buffer, source_w, source_h,
                                   0, 0,
                                   source_w, source_h,
                                   dest_buffer,
                                   dest_w, dest_h,
                                   dest_w, dest_h,
                                   0, 0);
  } else if (zoom_out_shift == 1) {
    buffer_copy_reduce_2_tiff_safe(source_buffer, source_w, source_h,
                                   0, 0,
                                   source_w, source_h,
                                   dest_buffer, dest_w, dest_h,
                                   dest_w, dest_h,
                                   0, 0,
                                   row_buffer);
  } else if (zoom_out_shift == 2 || zoom_out_shift == 3) {
    buffer_copy_reduce_max_8_tiff_safe(source_buffer, source_w, source_h,
                                       0, 0,
                                       source_w, source_h,
                                       dest_buffer, dest_w, dest_h,
                                       dest_w, dest_h,
                                       0, 0,
                                       zoom_out_shift,
                                       row_buffer);
  } else {
    buffer_copy_reduce_tiff_safe(source_buffer, source_w, source_h,
                                 0, 0,
                                 source_w, source_h,
                                 dest_buffer, dest_w, dest_h,
                                 dest_w, dest_h,
                                 0, 0,
                                 zoom_out_shift,
                                 row_buffer);
  }
}


#define SOURCE_TYPE TIFF_SOURCE_TYPE
#define NOREDUCE_FUNCNAME TIFF_NOREDUCE_FUNCNAME
#define NOREDUCE_COPY_EXPRESSION dest_buffer[dest_pixel]=(INT64)TIFFGetR(source_buffer[source_pixel]); \
                                 dest_buffer[dest_pixel]+=(INT64)TIFFGetG(source_buffer[source_pixel]) << G_SHIFT; \
                                 dest_buffer[dest_pixel]+=(INT64)TIFFGetB(source_buffer[source_pixel]) << B_SHIFT; \
                                 dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
#define REDUCE2_FUNCNAME TIFF_REDUCE2_FUNCNAME
#define REDUCE2_COPY_EXPRESSION row_buffer[ri]+=(INT64)TIFFGetR(source_buffer[source_pixel]); \
                                row_buffer[ri]+=(INT64)TIFFGetG(source_buffer[source_pixel]) << G_SHIFT_COMPACT; \
                                row_buffer[ri]+=(INT64)TIFFGetB(source_buffer[source_pixel]) << B_SHIFT_COMPACT;
#define REDUCE_MAX_8_FUNCNAME TIFF_REDUCE_MAX_8_FUNCNAME
#define REDUCE_MAX_8_COPY_EXPRESSION row_buffer[ri]+=(INT64)TIFFGetR(source_buffer[source_pixel]); \
                                     row_buffer[ri]+=(INT64)TIFFGetG(source_buffer[source_pixel]) << G_SHIFT_COMPACT; \
                                     row_buffer[ri]+=(INT64)TIFFGetB(source_buffer[source_pixel]) << B_SHIFT_COMPACT;
#define REDUCE_ALL_FUNCNAME TIFF_REDUCE_ALL_FUNCNAME
#define REDUCE_ALL_COPY_EXPRESSION row_buffer[ri*3]+=TIFFGetR(source_buffer[source_pixel]); \
                                   row_buffer[ri*3+1]+=TIFFGetG(source_buffer[source_pixel]); \
                                   row_buffer[ri*3+2]+=TIFFGetB(source_buffer[source_pixel]);
#include "buffer_manip_generic.hpp"
#undef SOURCE_TYPE
#undef NOREDUCE_FUNCNAME
#undef NOREDUCE_SOURCE_TYPE
#undef NOREDUCE_COPY_EXPRESSION
#undef REDUCE2_FUNCNAME
#undef REDUCE2_COPY_EXPRESSION
#undef REDUCE_MAX_8_FUNCNAME
#undef REDUCE_MAX_8_COPY_EXPRESSION
#undef REDUCE_ALL_FUNCNAME
#undef REDUCE_ALL_COPY_EXPRESSION

void buffer_copy_reduce_standard (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_visible, INT64 dest_h_visible,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out_shift,
                                 INT64* const row_buffer) {
  if (zoom_out_shift == 0) {
    buffer_copy_noreduce_standard_safe(source_buffer, source_w, source_h,
                                       source_start_x, source_start_y,
                                       source_copy_w, source_copy_h,
                                       dest_buffer,
                                       dest_w, dest_h,
                                       dest_w_visible, dest_h_visible,
                                       dest_start_x, dest_start_y);
  } else if (zoom_out_shift == 1) {
    buffer_copy_reduce_2_standard_safe(source_buffer, source_w, source_h,
                                       source_start_x, source_start_y,
                                       source_copy_w, source_copy_h,
                                       dest_buffer,
                                       dest_w, dest_h,
                                       dest_w_visible, dest_h_visible,
                                       dest_start_x, dest_start_y,
                                       row_buffer);
  } else if (zoom_out_shift == 2 || zoom_out_shift == 3) {
    buffer_copy_reduce_max_8_standard_safe(source_buffer, source_w, source_h,
                                           source_start_x, source_start_y,
                                           source_copy_w, source_copy_h,
                                           dest_buffer,
                                           dest_w, dest_h,
                                           dest_w_visible, dest_h_visible,
                                           dest_start_x, dest_start_y,
                                           zoom_out_shift,
                                           row_buffer);
  } else {
    buffer_copy_reduce_standard_safe(source_buffer, source_w, source_h,
                                     source_start_x, source_start_y,
                                     source_copy_w, source_copy_h,
                                     dest_buffer,
                                     dest_w, dest_h,
                                     dest_w_visible, dest_h_visible,
                                     dest_start_x, dest_start_y,
                                     zoom_out_shift,
                                     row_buffer);
  }
}

#define SOURCE_TYPE GENERIC_SOURCE_TYPE
#define NOREDUCE_FUNCNAME GENERIC_NOREDUCE_FUNCNAME
#define NOREDUCE_COPY_EXPRESSION dest_buffer[dest_pixel]=source_buffer[source_pixel];
#define REDUCE2_FUNCNAME GENERIC_REDUCE2_FUNCNAME
#define REDUCE2_COPY_EXPRESSION row_buffer[ri]+=(INT64)source_buffer[source_pixel] & R_MASK; \
                                row_buffer[ri]+=((INT64)source_buffer[source_pixel] & G_MASK) >> G_SHIFT << G_SHIFT_COMPACT; \
                                row_buffer[ri]+=((INT64)source_buffer[source_pixel] & B_MASK) >> B_SHIFT << B_SHIFT_COMPACT;
#define REDUCE_MAX_8_FUNCNAME GENERIC_REDUCE_MAX_8_FUNCNAME
#define REDUCE_MAX_8_COPY_EXPRESSION row_buffer[ri]+=((INT64)source_buffer[source_pixel] & R_MASK); \
                                     row_buffer[ri]+=((INT64)source_buffer[source_pixel] & G_MASK) >> G_SHIFT << G_SHIFT_COMPACT; \
                                     row_buffer[ri]+=((INT64)source_buffer[source_pixel] & B_MASK) >> B_SHIFT << B_SHIFT_COMPACT;
#define REDUCE_ALL_FUNCNAME GENERIC_REDUCE_ALL_FUNCNAME
#define REDUCE_ALL_COPY_EXPRESSION row_buffer[ri*3]+=(INT64)source_buffer[source_pixel] & R_MASK; \
                                   row_buffer[ri*3+1]+=((INT64)source_buffer[source_pixel] & G_MASK) >> G_SHIFT; \
                                   row_buffer[ri*3+2]+=((INT64)source_buffer[source_pixel] & B_MASK) >> B_SHIFT;
#include "buffer_manip_generic.hpp"
#undef SOURCE_TYPE
#undef NOREDUCE_FUNCNAME
#undef NOREDUCE_COPY_EXPRESSION
#undef REDUCE2_FUNCNAME
#undef REDUCE2_COPY_EXPRESSION
#undef REDUCE_MAX_8_FUNCNAME
#undef REDUCE_MAX_8_COPY_EXPRESSION
#undef REDUCE_ALL_FUNCNAME
#undef REDUCE_ALL_COPY_EXPRESSION

void buffer_copy_expand_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_visible, INT64 dest_h_visible,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_in_shift) {
  buffer_copy_expand_generic_safe (source_buffer, source_w, source_h,
                                   source_start_x, source_start_y,
                                   source_copy_w, source_copy_h,
                                   dest_buffer,
                                   dest_w, dest_h,
                                   dest_w_visible, dest_h_visible,
                                   dest_start_x, dest_start_y,
                                   zoom_in_shift);
}

void buffer_copy_expand_generic_safe (const PIXEL_RGBA* const source_buffer,
                                      INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_visible, INT64 dest_h_visible,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in_shift) {
  auto zoom_skip=1L << zoom_in_shift;
  for (INT64 sj=source_start_y, bdj=dest_start_y;
       sj < source_start_y+source_copy_h && sj < source_h;
       sj++, bdj+=zoom_skip) {
    for (INT64 dj=bdj; dj < bdj+zoom_skip && dj < dest_h_visible; dj++) {
      for (INT64 si=source_start_x, bdi=dest_start_x;
           si < source_start_x+source_copy_w && si < source_w;
           si++, bdi+=zoom_skip) {
        auto source_buffer_pixel=sj*source_w+si;
        auto src_data=source_buffer[source_buffer_pixel];
        for (INT64 di=bdi; di < bdi+zoom_skip && di < dest_w_visible; di++) {
          auto dest_pixel=dj*dest_w+di;
          dest_buffer[dest_pixel]=src_data;
        }
      }
    }
  }
}
