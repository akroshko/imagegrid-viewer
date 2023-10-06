// local headers
#include "../common.hpp"
// C++ headers
#include <memory>
// C headers
#include <cmath>
#include <cstring>
// C library headers
#include <tiffio.h>

typedef unsigned char (*rgb_extract)(unsigned char);

void buffer_copy_reduce_tiff (uint32_t* source_buffer, uint32_t w, uint32_t h,
                              unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT_T zoom_index) {
  // the loops in this function ensure memory is accessed sequentially
  // TODO: may wish to figure out an appropriate reduced size
  //       since 64 bit integers may be overkill
  auto row_buffer=std::make_unique<INT_T[]>(w_reduced*3);

  // TODO: this can probably be further optimized by passing in the
  // precomputed shift from elsewhere in the program
  INT_T block_average_shift=2*floor(log2(zoom_index));


  for (size_t j=0; j < h_reduced; j++) {
    std::memset((void*)row_buffer.get(),0,sizeof(INT_T)*w_reduced*3);
    for (size_t tj=j*zoom_index; tj < (j+1)*zoom_index; tj++) {
      for (size_t i=0; i < w_reduced; i++) {
        for (size_t ti=i*zoom_index; ti < (i+1)*zoom_index; ti++) {
          auto tiff_pixel=tj*w+ti;
          if ((ti < w) && (tj < h)) {
            row_buffer[i*3+0]+=TIFFGetR(source_buffer[tiff_pixel]);
            row_buffer[i*3+1]+=TIFFGetG(source_buffer[tiff_pixel]);
            row_buffer[i*3+2]+=TIFFGetB(source_buffer[tiff_pixel]);
          }
        }
      }
    }
    for (size_t i=0; i < w_reduced; i++) {
      auto rgb_pixel=j*w_reduced+i;
      dest_buffer[rgb_pixel*3]=(unsigned char)(row_buffer[i*3+0] >> block_average_shift);
      dest_buffer[rgb_pixel*3+1]=(unsigned char)(row_buffer[i*3+1] >> block_average_shift);
      dest_buffer[rgb_pixel*3+2]=(unsigned char)(row_buffer[i*3+2] >> block_average_shift);
    }
  }
}

// TODO: finish looping over only necessary pixels
void buffer_copy_reduce_generic (unsigned char* source_buffer, size_t w, size_t h,
                                 INT_T x_origin, INT_T y_origin,
                                 unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                                 INT_T zoom_index) {
  // TODO: zoom_index is not the index, since index is power of two
  //       fix!!!

  // di:=destination i
  // dj:=destination j
  // bsi:=source i at beginning of block
  // bsj:=source j at beginning of block
  // si:=source i
  // sj:=source j
  // etc

  // row buffer to copy to
  // TODO: may wish to figure out an appropriate reduced size since 64
  // bit integers are probably overkill for any forseeable need
  auto row_buffer=std::make_unique<INT_T[]>(w_reduced*3);

  // TODO: this can probably be further optimized by passing in the
  // precomputed shift from elsewhere in the program
  INT_T block_average_shift=2*floor(log2(zoom_index));

  // the loops in this function ensure memory is accessed sequentially
  // TODO: want to make sure I hit last one
  // these outer two loops hit all pixels in source image
  for (size_t bsj=0, dj=(y_origin/zoom_index);
       bsj < h;
       bsj+=zoom_index, dj++) {
    // zero out the row buffer each one of these
    std::memset((void*)row_buffer.get(),0,sizeof(INT_T)*w_reduced*3);
    // for testing
    // for (INT_T i=0; i < w_reduced*3; i++) {
    //   row_buffer[i]=0;
    // }
    for (size_t sj=bsj; sj < bsj+zoom_index; sj++) {
      for (size_t bsi=0; bsi < w; bsi+=zoom_index) {
        for (size_t si=bsi; si < bsi+zoom_index; si++) {
          auto source_pixel=sj*w+si;
          auto row_buffer_pixel=((x_origin+si)/zoom_index);
          // work out pixel math more carefuly and/or test with an
          // assert
          if (row_buffer_pixel < w_reduced && si < w && sj < h) {
            row_buffer[row_buffer_pixel*3]+=source_buffer[source_pixel*3];
            row_buffer[row_buffer_pixel*3+1]+=source_buffer[source_pixel*3+1];
            row_buffer[row_buffer_pixel*3+2]+=source_buffer[source_pixel*3+2];
          }
        }
      }
    }
    for (size_t di=x_origin/zoom_index; di < w_reduced; di++) {
      // TODO: possibly assert instead of testing for overflow
      if (di < w_reduced && dj < h_reduced) {
        auto dest_pixel=dj*w_reduced+di;
        dest_buffer[dest_pixel*3]=(unsigned char)(row_buffer[di*3] >> block_average_shift);
        dest_buffer[dest_pixel*3+1]=(unsigned char)(row_buffer[di*3+1] >> block_average_shift);
        dest_buffer[dest_pixel*3+2]=(unsigned char)(row_buffer[di*3+2] >> block_average_shift);
      }
    }
  }
}

void buffer_copy_expand_generic (unsigned char* source_buffer, size_t w, size_t h,
                                 INT_T x_origin, INT_T y_origin,
                                 unsigned char* dest_buffer, size_t w_expanded, size_t h_expanded,
                                 INT_T zoom_index) {
  auto zoom_skip=zoom_index;
  auto dest_i_beg=x_origin*zoom_skip;
  auto dest_j_beg=y_origin*zoom_skip;
  auto dest_i_end=(x_origin+w)*zoom_skip;
  auto dest_j_end=(x_origin+w)*zoom_skip;
  for (size_t bdj=dest_j_beg, sj=0;
       (bdj < h_expanded && bdj < dest_j_end);
       bdj+=zoom_skip, sj++) {
    for (size_t dj=bdj; dj < bdj+zoom_skip; dj++) {
      for (size_t bdi=dest_i_beg, si=0;
           (bdi < w_expanded && bdi < dest_i_end);
           bdi+=zoom_skip, si++) {
        // not valid source pixels
        if (si < w && sj < h) {
          auto source_buffer_pixel=sj*w+si;
          auto src_data_0=source_buffer[source_buffer_pixel*3];
          auto src_data_1=source_buffer[source_buffer_pixel*3+1];
          auto src_data_2=source_buffer[source_buffer_pixel*3+2];
          for (size_t di=bdi; di < bdi+zoom_skip; di++) {
            // not valid dest pixels
            if (di < w_expanded && dj < h_expanded) {
              auto dest_pixel=dj*w_expanded+di;
              ((unsigned char *)dest_buffer)[dest_pixel*3]=src_data_0;
              ((unsigned char *)dest_buffer)[dest_pixel*3+1]=src_data_1;
              ((unsigned char *)dest_buffer)[dest_pixel*3+2]=src_data_2;
            }
          }
        }
      }
    }
  }
}
