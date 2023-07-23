// local headers
#include "../debug.hpp"
#include "../error.hpp"
#include "../types.hpp"
// C++ headers
#include <cmath>
#include <memory>
// C headers
#include <png.h>
#include <tiffio.h>

typedef unsigned char (*rgb_extract)(unsigned char);

void buffer_copy_reduce_tiff (uint32* source_buffer, uint32 w, uint32 h,
                              unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                              INT_T zoom_index) {
  // the loops in this function ensure memory is accessed sequentially
  // TODO: may wish to figure out an appropriate reduced size
  //       since 64 bit integers may be overkill
  auto row_buffer=std::make_unique<INT_T[]>(w_reduced*3);

  for (size_t j=0; j < h_reduced; j++) {
    for (size_t i=0; i < w_reduced*3; i++) {
      row_buffer[i]=0;
    }
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
    INT_T number_sum=zoom_index*zoom_index;
    for (size_t i=0; i < w_reduced; i++) {
      auto rgb_pixel=j*w_reduced+i;
      (dest_buffer)[rgb_pixel*3]=(unsigned char)round((FLOAT_T)row_buffer[i*3+0]/(FLOAT_T)number_sum);
      (dest_buffer)[rgb_pixel*3+1]=(unsigned char)round((FLOAT_T)row_buffer[i*3+1]/(FLOAT_T)number_sum);
      (dest_buffer)[rgb_pixel*3+2]=(unsigned char)round((FLOAT_T)row_buffer[i*3+2]/(FLOAT_T)number_sum);
    }
  }
}

void buffer_copy_reduce_generic (unsigned char* source_buffer, size_t w, size_t h,
                                 unsigned char* dest_buffer, size_t w_reduced, size_t h_reduced,
                                 INT_T zoom_index) {
  // the loops in this function ensure memory is accessed sequentially
  // TODO: may wish to figure out an appropriate reduced size
  //       since 64 bit integers may be overkill
  auto row_buffer=std::make_unique<INT_T[]>(w_reduced*3);

  for (size_t j=0; j < h_reduced; j++) {
    for (size_t i=0; i < w_reduced*3; i++) {
      row_buffer[i]=0;
    }
    for (size_t tj=j*zoom_index; tj < (j+1)*zoom_index; tj++) {
      for (size_t i=0; i < w_reduced; i++) {
        for (size_t ti=i*zoom_index; ti < (i+1)*zoom_index; ti++) {
          auto source_pixel=tj*w+ti;
          if ((ti < w) && (tj < h)) {
            row_buffer[i*3+0]+=source_buffer[source_pixel*3];
            row_buffer[i*3+1]+=source_buffer[source_pixel*3+1];
            row_buffer[i*3+2]+=source_buffer[source_pixel*3+2];
          }
        }
      }
    }
    INT_T number_sum=zoom_index*zoom_index;
    for (size_t i=0; i < w_reduced; i++) {
      auto rgb_pixel=j*w_reduced+i;
      (dest_buffer)[rgb_pixel*3]=(unsigned char)round((FLOAT_T)row_buffer[i*3+0]/(FLOAT_T)number_sum);
      (dest_buffer)[rgb_pixel*3+1]=(unsigned char)round((FLOAT_T)row_buffer[i*3+1]/(FLOAT_T)number_sum);
      (dest_buffer)[rgb_pixel*3+2]=(unsigned char)round((FLOAT_T)row_buffer[i*3+2]/(FLOAT_T)number_sum);
    }
  }
}
