// local headers
#include "../common.hpp"
#include "buffer_manip.hpp"
// C++ headers
#include <memory>
// C headers
#include <cmath>
#include <cstring>
#include <cstdint>
// C library headers
#include <tiffio.h>

typedef unsigned char (*rgb_extract)(unsigned char);

#define G_SHIFT 8
#define B_SHIFT 16
#define DEFAULT_ALPHA 0xFF000000

void buffer_copy_reduce_tiff (uint32_t* source_buffer, uint32_t w, uint32_t h,
                              PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT64 zoom_out) {
  buffer_copy_reduce_tiff_safe(source_buffer, w, h,
                               dest_buffer, w_reduced, h_reduced,
                               zoom_out);
}

void buffer_copy_reduce_tiff_safe (uint32_t* source_buffer, uint32_t w, uint32_t h,
                                   PIXEL_RGBA* dest_buffer, size_t w_reduced, size_t h_reduced,
                                   INT64 zoom_out) {
  // the loops in this function ensure memory is accessed sequentially
  // TODO: may wish to figure out an appropriate reduced size
  //       since 64 bit integers may be overkill
  auto row_buffer=std::make_unique<INT64[]>(w_reduced*3);

  // TODO: this can probably be further optimized by passing in the
  // precomputed shift from elsewhere in the program
  INT64 block_average_shift=2*floor(log2(zoom_out));

  for (size_t j=0; j < h_reduced; j++) {
    std::memset((void*)row_buffer.get(),0,sizeof(INT64)*w_reduced*3);
    for (size_t tj=j*zoom_out; tj < (j+1)*zoom_out; tj++) {
      for (size_t i=0; i < w_reduced; i++) {
        for (size_t ti=i*zoom_out; ti < (i+1)*zoom_out; ti++) {
          auto tiff_pixel=tj*w+ti;
          if ((ti < w) && (tj < h)) {
            row_buffer[i*3]+=TIFFGetR(source_buffer[tiff_pixel]);
            row_buffer[i*3+1]+=TIFFGetG(source_buffer[tiff_pixel]);
            row_buffer[i*3+2]+=TIFFGetB(source_buffer[tiff_pixel]);
          }
        }
      }
    }
    for (size_t i=0; i < w_reduced; i++) {
      auto rgba_pixel=j*w_reduced+i;
      dest_buffer[rgba_pixel]=(row_buffer[i*3] >> block_average_shift);
      dest_buffer[rgba_pixel]+=((row_buffer[i*3+1] >> block_average_shift) << G_SHIFT);
      dest_buffer[rgba_pixel]+=((row_buffer[i*3+2] >> block_average_shift) << B_SHIFT);
      dest_buffer[rgba_pixel]+=DEFAULT_ALPHA;
    }
  }
}

void buffer_copy_reduce_generic (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out) {
  buffer_copy_reduce_generic_safe(source_buffer, source_w, source_h,
                                  source_start_x, source_start_y,
                                  source_copy_w, source_copy_h,
                                  dest_buffer, dest_w, dest_h,
                                  dest_start_x, dest_start_y,
                                  zoom_out);
}

void buffer_copy_reduce_generic_safe (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_out) {
  // di:=destination i
  // dj:=destination j
  // bsi:=source i at beginning of block
  // bsj:=source j at beginning of block
  // si:=source i
  // sj:=source j
  // ri:=row buffer i
  // etc

  // row buffer to copy to
  // TODO: may wish to figure out an appropriate reduced size since 64
  // bit integers are probably overkill for any forseeable need
  auto row_buffer=std::make_unique<INT64[]>((source_copy_w/zoom_out)*3);

  // TODO: this can probably be further optimized by passing in the
  // precomputed shift from elsewhere in the program
  INT64 block_average_shift=2*floor(log2(zoom_out));

  // the loops in this function ensure memory is accessed sequentially
  // TODO: want to make sure I hit last one
  // these outer two loops hit all pixels in source image
  for (INT64 bsj=source_start_y, dj=dest_start_y;
       bsj < source_start_y+source_copy_h;
       bsj+=zoom_out, dj++) {
    // zero out the row buffer each one of these
    std::memset((void*)row_buffer.get(),0,sizeof(INT64)*(source_copy_w/zoom_out)*3);
    for (INT64 sj=bsj; sj < bsj+zoom_out; sj++) {
      for (INT64 bsi=source_start_x, ri=0;
           bsi < source_start_x+source_copy_w;
           bsi+=zoom_out, ri++) {
        for (INT64 si=bsi; si < bsi+zoom_out; si++) {
          auto source_pixel=sj*source_w+si;
          // auto row_buffer_pixel=(si-source_start_x)/zoom_out;
          // work out pixel math more carefuly so we don't have to do
          // this check every copy, test with asserts
          if (ri < (source_copy_w/zoom_out) && si < source_w && sj < source_h) {
            row_buffer[ri*3]+=(source_buffer[source_pixel] & 0xFF);
            row_buffer[ri*3+1]+=((source_buffer[source_pixel] & 0xFF00) >> G_SHIFT);
            row_buffer[ri*3+2]+=((source_buffer[source_pixel] & 0xFF0000) >> B_SHIFT);
          }
        }
      }
    }
    for (INT64 di=dest_start_x, ri=0; di < dest_start_x+(source_copy_w/zoom_out); di++, ri++) {
      if (ri < (source_copy_w/zoom_out) && di < dest_w && dj < dest_h) {
        auto dest_pixel=dj*dest_w+di;
        dest_buffer[dest_pixel]=(row_buffer[ri*3] >> block_average_shift);
        dest_buffer[dest_pixel]+=((row_buffer[ri*3+1] >> block_average_shift) << G_SHIFT);
        dest_buffer[dest_pixel]+=((row_buffer[ri*3+2] >> block_average_shift) << B_SHIFT);
        dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}


void buffer_copy_expand_generic (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_in) {
  buffer_copy_expand_generic_safe (source_buffer, source_w, source_h,
                                   source_start_x, source_start_y,
                                   source_copy_w, source_copy_h,
                                   dest_buffer, dest_w, dest_h,
                                   dest_start_x, dest_start_y,
                                   zoom_in);
}

void buffer_copy_expand_generic_safe (PIXEL_RGBA* source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in) {
  auto zoom_skip=zoom_in;
  for (INT64 bdj=dest_start_y, sj=source_start_y;
       sj < source_start_y+source_copy_h;
       bdj+=zoom_skip, sj++) {
    for (INT64 dj=bdj; dj < bdj+zoom_skip; dj++) {
      for (INT64 bdi=dest_start_x, si=source_start_x;
           si < source_start_x+source_copy_w;
           bdi+=zoom_skip, si++) {
        // not valid source pixels
        if (si >= source_start_x && si < source_w && sj >= source_start_y && sj < source_h) {
          auto source_buffer_pixel=sj*source_w+si;
          auto src_data=source_buffer[source_buffer_pixel];
          for (INT64 di=bdi; di < bdi+zoom_skip; di++) {
            // not valid dest pixels
            if (di < dest_w && dj < dest_h) {
              auto dest_pixel=dj*dest_w+di;
              dest_buffer[dest_pixel]=src_data;
            }
          }
        }
      }
    }
  }
}
