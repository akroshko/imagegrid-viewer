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

#define G_SHIFT 8L
#define B_SHIFT 16L
#define R_MASK_COMPACT 0x00000000FFFFL
#define G_SHIFT_COMPACT 16L
#define G_MASK_COMPACT 0x0000FFFF0000L
#define B_SHIFT_COMPACT 32L
#define B_MASK_COMPACT 0xFFFF00000000L

#define DEFAULT_ALPHA 0xFF000000L

void buffer_copy_reduce_tiff (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                              PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                              INT64 zoom_out_shift,
                              INT64* row_buffer) {
  if (zoom_out_shift == 0) {
    buffer_copy_noreduce_tiff_safe(source_buffer, source_w, source_h,
                                   dest_buffer, dest_w, dest_h);
  } else if (zoom_out_shift == 1) {
    buffer_copy_reduce_2_tiff_safe(source_buffer, source_w, source_h,
                                   dest_buffer, dest_w, dest_h,
                                   row_buffer);
  } else if (zoom_out_shift == 2 || zoom_out_shift == 3) {
    buffer_copy_reduce_max_8_tiff_safe(source_buffer, source_w, source_h,
                                       dest_buffer, dest_w, dest_h,
                                       zoom_out_shift,
                                       row_buffer);
  } else {
    buffer_copy_reduce_tiff_safe(source_buffer, source_w, source_h,
                                 dest_buffer, dest_w, dest_h,
                                 zoom_out_shift,
                                 row_buffer);
  }
}

void buffer_copy_noreduce_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                     PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h) {
  for (INT64 sj=0, dj=0; dj < dest_h; sj++, dj++) {
    for (INT64 si=0, di=0; di < dest_w; si++, di++) {
      auto rgba_pixel=dj*dest_w+di;
      auto tiff_pixel=sj*source_w+si;
      if (si < source_w && sj < source_h) {
        dest_buffer[rgba_pixel]=TIFFGetR(source_buffer[tiff_pixel]);
        dest_buffer[rgba_pixel]+=(TIFFGetG(source_buffer[tiff_pixel]) << G_SHIFT);
        dest_buffer[rgba_pixel]+=(TIFFGetB(source_buffer[tiff_pixel]) << B_SHIFT);
        dest_buffer[rgba_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}

// this is seperate from buffer_copy_reduce_max_8_tiff_safe because I
// will definitely have an optimized reduce-by-2
void buffer_copy_reduce_2_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                     PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                     INT64* row_buffer) {
  // the loops in this function ensure memory is accessed sequentially
  auto block_average_shift=2;
  for (INT64 dj=0; dj < dest_h; dj++) {
    std::memset((void*)row_buffer,0,sizeof(INT64)*dest_w);
    for (INT64 sj=0; sj < 2; sj++) {
      for (INT64 di=0; di < dest_w; di++) {
        for (INT64 si=0; si < 2; si++) {
          auto tj=2*dj+sj;
          auto ti=2*di+si;
          auto tiff_pixel=tj*source_w+ti;
          if ((ti < source_w) && (tj < source_h)) {
            row_buffer[di]+=TIFFGetR(source_buffer[tiff_pixel]);
            row_buffer[di]+=((INT64)TIFFGetG(source_buffer[tiff_pixel]) << G_SHIFT_COMPACT);
            row_buffer[di]+=((INT64)TIFFGetB(source_buffer[tiff_pixel]) << B_SHIFT_COMPACT);
          }
        }
      }
    }
    for (INT64 di=0; di < dest_w; di++) {
      auto rgba_pixel=dj*dest_w+di;
      dest_buffer[rgba_pixel]=((row_buffer[di] & R_MASK_COMPACT) >> block_average_shift);
      dest_buffer[rgba_pixel]+=((((row_buffer[di] & G_MASK_COMPACT) >> G_SHIFT_COMPACT) >> block_average_shift) << G_SHIFT);
      dest_buffer[rgba_pixel]+=((((row_buffer[di] & B_MASK_COMPACT) >> B_SHIFT_COMPACT) >> block_average_shift) << B_SHIFT);
      dest_buffer[rgba_pixel]+=DEFAULT_ALPHA;
    }
  }
}


void buffer_copy_reduce_max_8_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                         PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                         INT64 zoom_out_shift,
                                         INT64* row_buffer) {
  // the loops in this function ensure memory is accessed sequentially
  auto block_average_shift=zoom_out_shift*2;
  auto zoom_out=1L << zoom_out_shift;
  for (INT64 dj=0; dj < dest_h; dj++) {
    std::memset((void*)row_buffer,0,sizeof(INT64)*dest_w);
    for (INT64 sj=0; sj < zoom_out; sj++) {
      for (INT64 di=0; di < dest_w; di++) {
        for (INT64 si=0; si < zoom_out; si++) {
          auto tj=zoom_out*dj+sj;
          auto ti=zoom_out*di+si;
          auto tiff_pixel=tj*source_w+ti;
          if ((ti < source_w) && (tj < source_h)) {
            row_buffer[di]+=TIFFGetR(source_buffer[tiff_pixel]);
            row_buffer[di]+=((INT64)TIFFGetG(source_buffer[tiff_pixel]) << G_SHIFT_COMPACT);
            row_buffer[di]+=((INT64)TIFFGetB(source_buffer[tiff_pixel]) << B_SHIFT_COMPACT);
          }
        }
      }
    }
    for (INT64 di=0; di < dest_w; di++) {
      auto rgba_pixel=dj*dest_w+di;
      dest_buffer[rgba_pixel]=((row_buffer[di] & R_MASK_COMPACT) >> block_average_shift);
      dest_buffer[rgba_pixel]+=((((row_buffer[di] & G_MASK_COMPACT) >> G_SHIFT_COMPACT) >> block_average_shift) << G_SHIFT);
      dest_buffer[rgba_pixel]+=((((row_buffer[di] & B_MASK_COMPACT) >> B_SHIFT_COMPACT) >> block_average_shift) << B_SHIFT);
      dest_buffer[rgba_pixel]+=DEFAULT_ALPHA;
    }
  }
}

void buffer_copy_reduce_tiff_safe (const uint32_t* const source_buffer, INT64 source_w, INT64 source_h,
                                   PIXEL_RGBA* dest_buffer, INT64 dest_w, INT64 dest_h,
                                   INT64 zoom_out_shift,
                                   INT64* row_buffer) {
  // the loops in this function ensure memory is accessed sequentially
  auto block_average_shift=2*zoom_out_shift;
  auto zoom_out=1L << zoom_out_shift;

  for (INT64 dj=0; dj < dest_h; dj++) {
    std::memset((void*)row_buffer,0,sizeof(INT64)*dest_w*3);
    for (INT64 tj=dj*zoom_out; tj < (dj+1)*zoom_out; tj++) {
      for (INT64 di=0; di < dest_w; di++) {
        for (INT64 ti=di*zoom_out; ti < (di+1)*zoom_out; ti++) {
          auto tiff_pixel=tj*source_w+ti;
          if ((ti < source_w) && (tj < source_h)) {
            row_buffer[di*3]+=TIFFGetR(source_buffer[tiff_pixel]);
            row_buffer[di*3+1]+=TIFFGetG(source_buffer[tiff_pixel]);
            row_buffer[di*3+2]+=TIFFGetB(source_buffer[tiff_pixel]);
          }
        }
      }
    }
    for (INT64 i=0; i < dest_w; i++) {
      auto rgba_pixel=dj*dest_w+i;
      dest_buffer[rgba_pixel]=(row_buffer[i*3] >> block_average_shift);
      dest_buffer[rgba_pixel]+=((row_buffer[i*3+1] >> block_average_shift) << G_SHIFT);
      dest_buffer[rgba_pixel]+=((row_buffer[i*3+2] >> block_average_shift) << B_SHIFT);
      dest_buffer[rgba_pixel]+=DEFAULT_ALPHA;
    }
  }
}

void buffer_copy_reduce_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_limit, INT64 dest_h_limit,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_out_shift,
                                 INT64* row_buffer) {
  if (zoom_out_shift == 0) {
    buffer_copy_noreduce_generic_safe(source_buffer, source_w, source_h,
                                      source_start_x, source_start_y,
                                      source_copy_w, source_copy_h,
                                      dest_buffer,
                                      dest_w, dest_h,
                                      dest_w_limit, dest_h_limit,
                                      dest_start_x, dest_start_y);
  } else {
    buffer_copy_reduce_generic_safe(source_buffer, source_w, source_h,
                                    source_start_x, source_start_y,
                                    source_copy_w, source_copy_h,
                                    dest_buffer,
                                    dest_w, dest_h,
                                    dest_w_limit, dest_h_limit,
                                    dest_start_x, dest_start_y,
                                    zoom_out_shift,
                                    row_buffer);
  }
}

void buffer_copy_noreduce_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                        INT64 source_start_x, INT64 source_start_y,
                                        INT64 source_copy_w, INT64 source_copy_h,
                                        PIXEL_RGBA* dest_buffer,
                                        INT64 dest_w, INT64 dest_h,
                                        INT64 dest_w_limit, INT64 dest_h_limit,
                                        INT64 dest_start_x, INT64 dest_start_y) {
  for (INT64 sj=source_start_y, dj=dest_start_y;
       sj < source_start_y+source_copy_h;
       sj++, dj++) {
    if (dj < dest_h_limit) {
      for (INT64 si=source_start_x, di=dest_start_x;
           si < source_start_x+source_copy_w;
           si++, di++) {
        if (si < source_w && di < dest_w_limit &&
            sj < source_h && dj < dest_h_limit) {
          auto source_pixel=sj*source_w+si;
          auto dest_pixel=dj*dest_w+di;
          dest_buffer[dest_pixel]=source_buffer[source_pixel];
        }
      }
    }
  }
}

void buffer_copy_reduce_generic_safe (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_limit, INT64 dest_h_limit,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_out_shift,
                                      INT64* row_buffer) {
  // di:=destination i
  // dj:=destination j
  // bsi:=source i at beginning of block
  // bsj:=source j at beginning of block
  // si:=source i
  // sj:=source j
  // ri:=row buffer i
  // etc

  // row buffer to copy to
  INT64 block_average_shift=2*zoom_out_shift;
  auto zoom_out=1L << zoom_out_shift;

  // the loops in this function ensure memory is accessed sequentially
  // TODO: want to make sure I hit last one
  // these outer two loops hit all pixels in source image
  for (INT64 bsj=source_start_y, dj=dest_start_y;
       bsj < source_start_y+source_copy_h;
       bsj+=zoom_out, dj++) {
    // zero out the row buffer each one of these
    std::memset((void*)row_buffer,0,sizeof(INT64)*(source_copy_w/zoom_out)*3);
    for (INT64 sj=bsj; sj < bsj+zoom_out; sj++) {
      if (dj < dest_h_limit) {
        for (INT64 bsi=source_start_x, ri=0;
             bsi < source_start_x+source_copy_w;
             bsi+=zoom_out, ri++) {
          for (INT64 si=bsi; si < bsi+zoom_out; si++) {
            auto source_pixel=sj*source_w+si;
            // work out pixel math more carefuly so we don't have to do
            // this check every copy, test with asserts
            if (ri < (source_copy_w/zoom_out) && ri < (dest_w_limit-dest_start_x) &&
                si < source_w && sj < source_h) {
              row_buffer[ri*3]+=(source_buffer[source_pixel] & 0xFF);
              row_buffer[ri*3+1]+=((source_buffer[source_pixel] & 0xFF00) >> G_SHIFT);
              row_buffer[ri*3+2]+=((source_buffer[source_pixel] & 0xFF0000) >> B_SHIFT);
            }
          }
        }
      }
    }
    for (INT64 di=dest_start_x, ri=0; di < dest_start_x+(source_copy_w/zoom_out); di++, ri++) {
      if (ri < (source_copy_w/zoom_out) &&
          di < dest_w_limit && dj < dest_h_limit &&
          di < dest_w && dj < dest_h) {
        auto dest_pixel=dj*dest_w+di;
        dest_buffer[dest_pixel]=(row_buffer[ri*3] >> block_average_shift);
        dest_buffer[dest_pixel]+=((row_buffer[ri*3+1] >> block_average_shift) << G_SHIFT);
        dest_buffer[dest_pixel]+=((row_buffer[ri*3+2] >> block_average_shift) << B_SHIFT);
        dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}

void buffer_copy_expand_generic (const PIXEL_RGBA* const source_buffer, INT64 source_w, INT64 source_h,
                                 INT64 source_start_x, INT64 source_start_y,
                                 INT64 source_copy_w, INT64 source_copy_h,
                                 PIXEL_RGBA* dest_buffer,
                                 INT64 dest_w, INT64 dest_h,
                                 INT64 dest_w_limit, INT64 dest_h_limit,
                                 INT64 dest_start_x, INT64 dest_start_y,
                                 INT64 zoom_in_shift) {
  buffer_copy_expand_generic_safe (source_buffer, source_w, source_h,
                                   source_start_x, source_start_y,
                                   source_copy_w, source_copy_h,
                                   dest_buffer,
                                   dest_w, dest_h,
                                   dest_w_limit, dest_h_limit,
                                   dest_start_x, dest_start_y,
                                   zoom_in_shift);
}

void buffer_copy_expand_generic_safe (const PIXEL_RGBA* const source_buffer,
                                      INT64 source_w, INT64 source_h,
                                      INT64 source_start_x, INT64 source_start_y,
                                      INT64 source_copy_w, INT64 source_copy_h,
                                      PIXEL_RGBA* dest_buffer,
                                      INT64 dest_w, INT64 dest_h,
                                      INT64 dest_w_limit, INT64 dest_h_limit,
                                      INT64 dest_start_x, INT64 dest_start_y,
                                      INT64 zoom_in_shift) {
  auto zoom_skip=1L << zoom_in_shift;
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
            if (di < dest_w_limit && dj < dest_h_limit) {
              auto dest_pixel=dj*dest_w+di;
              dest_buffer[dest_pixel]=src_data;
            }
          }
        }
      }
    }
  }
}
