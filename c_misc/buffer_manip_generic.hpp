#ifdef NOREDUCE_FUNCNAME
void (NOREDUCE_FUNCNAME) (SOURCE_TYPE source_buffer,
                          const BufferPixelSize& source_size,
                          const BufferPixelCoordinate& source_start,
                          const BufferPixelSize& source_copy_size,
                          PIXEL_RGBA* const dest_buffer,
                          const BufferPixelSize& dest_size,
                          const BufferPixelSize& dest_size_visible,
                          const BufferPixelCoordinate& dest_start) {
  auto source_w=source_size.w();
  auto source_h=source_size.h();
  auto source_start_x=source_start.x();
  auto source_start_y=source_start.y();
  auto source_copy_w=source_copy_size.w();
  auto source_copy_h=source_copy_size.h();
  auto dest_w=dest_size.w();
  // auto dest_h=dest_size.h();
  auto dest_w_visible=dest_size_visible.w();
  auto dest_h_visible=dest_size_visible.h();
  auto dest_start_x=dest_start.x();
  auto dest_start_y=dest_start.y();

  for (INT64 sj=source_start_y, dj=dest_start_y;
       sj < source_start_y+source_copy_h;
       sj++, dj++) {
    if (dj < dest_h_visible) {
      for (INT64 si=source_start_x, di=dest_start_x;
           si < source_start_x+source_copy_w;
           si++, di++) {
        if (di < dest_w_visible && dj < dest_h_visible) {
          auto dest_pixel=dj*dest_w+di;
          if (si < source_w && sj < source_h &&
              di < dest_w_visible && dj < dest_h_visible) {
            auto source_pixel=sj*source_w+si;
            NOREDUCE_COPY_EXPRESSION;
          }
        }
      }
    }
  }
}
#endif

#ifdef REDUCE2_FUNCNAME
void (REDUCE2_FUNCNAME) (SOURCE_TYPE source_buffer,
                         const BufferPixelSize& source_size,
                         const BufferPixelCoordinate& source_start,
                         const BufferPixelSize& source_copy_size,
                         PIXEL_RGBA* dest_buffer,
                         const BufferPixelSize& dest_size,
                         const BufferPixelSize& dest_size_visible,
                         const BufferPixelCoordinate& dest_start,
                         INT64* const row_buffer) {
  auto source_w=source_size.w();
  // auto source_h=source_size.h();
  auto source_start_x=source_start.x();
  auto source_start_y=source_start.y();
  auto source_copy_w=source_copy_size.w();
  auto source_copy_h=source_copy_size.h();
  auto dest_w=dest_size.w();
  // auto dest_h=dest_size.h();
  auto dest_w_visible=dest_size_visible.w();
  auto dest_h_visible=dest_size_visible.h();
  auto dest_start_x=dest_start.x();
  auto dest_start_y=dest_start.y();

  // di:=destination i
  // dj:=destination j
  // bsi:=source i at beginning of block
  // bsj:=source j at beginning of block
  // si:=source i
  // sj:=source j
  // ri:=row buffer i
  // etc

  // row buffer to copy to
  INT64 block_average_shift=2;

  // the loops in this function ensure memory is accessed sequentially
  // TODO: want to make sure I hit last one
  // these outer two loops hit all pixels in source image
  for (INT64 bsj=source_start_y, dj=dest_start_y;
       bsj < source_start_y+source_copy_h;
       bsj+=2, dj++) {
    // zero out the row buffer each one of these
    std::memset((void*)row_buffer,0,sizeof(INT64)*(source_copy_w/2));
    for (INT64 sj=bsj; sj < bsj+2 && sj < source_start_y+source_copy_h; sj++) {
      if (dj < dest_h_visible) {
        for (INT64 bsi=source_start_x, ri=0;
             bsi < source_start_x+source_copy_w;
             bsi+=2, ri++) {
          for (INT64 si=bsi; si < bsi+2 && si < source_start_x+source_copy_w && si < source_w; si++) {
            auto source_pixel=sj*source_w+si;
            // work out pixel math more carefully so we don't have to do
            // this check every copy, test with asserts
            if (ri < (source_copy_w/2) && ri < (dest_w_visible-dest_start_x)) {
              REDUCE2_COPY_EXPRESSION;
            }
          }
        }
      }
    }
    for (INT64 di=dest_start_x, ri=0; di < dest_start_x+(source_copy_w/2); di++, ri++) {
      if (ri < (source_copy_w/2) &&
          di < dest_w_visible && dj < dest_h_visible) {
        auto dest_pixel=dj*dest_w+di;
        dest_buffer[dest_pixel]=(row_buffer[ri] & R_MASK_COMPACT) >> block_average_shift;
        dest_buffer[dest_pixel]+=(row_buffer[ri] & G_MASK_COMPACT) >> G_SHIFT_COMPACT >> block_average_shift << G_SHIFT;
        dest_buffer[dest_pixel]+=(row_buffer[ri] & B_MASK_COMPACT) >> B_SHIFT_COMPACT >> block_average_shift << B_SHIFT;
        dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}
#endif

#ifdef REDUCE_MAX_8_FUNCNAME
void (REDUCE_MAX_8_FUNCNAME) (SOURCE_TYPE source_buffer,
                              const BufferPixelSize& source_size,
                              const BufferPixelCoordinate& source_start,
                              const BufferPixelSize& source_copy_size,
                              PIXEL_RGBA* dest_buffer,
                              const BufferPixelSize& dest_size,
                              const BufferPixelSize& dest_size_visible,
                              const BufferPixelCoordinate& dest_start,
                              INT64 zoom_out_shift,
                              INT64* const row_buffer) {
  auto source_w=source_size.w();
  // auto source_h=source_size.h();
  auto source_start_x=source_start.x();
  auto source_start_y=source_start.y();
  auto source_copy_w=source_copy_size.w();
  auto source_copy_h=source_copy_size.h();
  auto dest_w=dest_size.w();
  // auto dest_h=dest_size.h();
  auto dest_w_visible=dest_size_visible.w();
  auto dest_h_visible=dest_size_visible.h();
  auto dest_start_x=dest_start.x();
  auto dest_start_y=dest_start.y();

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
  INT64 zoom_out=1L << zoom_out_shift;

  // the loops in this function ensure memory is accessed sequentially
  // TODO: want to make sure I hit last one
  // these outer two loops hit all pixels in source image
  for (INT64 bsj=source_start_y, dj=dest_start_y;
       bsj < source_start_y+source_copy_h;
       bsj+=zoom_out, dj++) {
    // zero out the row buffer each one of these
    std::memset((void*)row_buffer,0,sizeof(INT64)*(source_copy_w/zoom_out));
    for (INT64 sj=bsj; sj < bsj+zoom_out && sj < source_start_y+source_copy_h; sj++) {
      if (dj < dest_h_visible) {
        for (INT64 bsi=source_start_x, ri=0;
             bsi < source_start_x+source_copy_w;
             bsi+=zoom_out, ri++) {
          for (INT64 si=bsi; si < bsi+zoom_out && si < source_start_x+source_copy_w && si < source_w; si++) {
            auto source_pixel=sj*source_w+si;
            // work out pixel math more carefully so we don't have to do
            // this check every copy, test with asserts
            if (ri < (source_copy_w/zoom_out) && ri < (dest_w_visible-dest_start_x)) {
              REDUCE_MAX_8_COPY_EXPRESSION;
            }
          }
        }
      }
    }
    for (INT64 di=dest_start_x, ri=0; di < dest_start_x+(source_copy_w/zoom_out); di++, ri++) {
      if (ri < (source_copy_w/zoom_out) &&
          di < dest_w_visible && dj < dest_h_visible) {
        auto dest_pixel=dj*dest_w+di;
        dest_buffer[dest_pixel]=(row_buffer[ri] & R_MASK_COMPACT) >> block_average_shift;
        dest_buffer[dest_pixel]+=(row_buffer[ri] & G_MASK_COMPACT) >> G_SHIFT_COMPACT >> block_average_shift << G_SHIFT;
        dest_buffer[dest_pixel]+=(row_buffer[ri] & B_MASK_COMPACT) >> B_SHIFT_COMPACT >> block_average_shift << B_SHIFT;
        dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}
#endif

#ifdef REDUCE_ALL_FUNCNAME
void (REDUCE_ALL_FUNCNAME) (SOURCE_TYPE source_buffer,
                            const BufferPixelSize& source_size,
                            const BufferPixelCoordinate& source_start,
                            const BufferPixelSize& source_copy_size,
                            PIXEL_RGBA* dest_buffer,
                            const BufferPixelSize& dest_size,
                            const BufferPixelSize& dest_size_visible,
                            const BufferPixelCoordinate& dest_start,
                            INT64 zoom_out_shift,
                            INT64* const row_buffer) {
  auto source_w=source_size.w();
  // auto source_h=source_size.h();
  auto source_start_x=source_start.x();
  auto source_start_y=source_start.y();
  auto source_copy_w=source_copy_size.w();
  auto source_copy_h=source_copy_size.h();
  auto dest_w=dest_size.w();
  // auto dest_h=dest_size.h();
  auto dest_w_visible=dest_size_visible.w();
  auto dest_h_visible=dest_size_visible.h();
  auto dest_start_x=dest_start.x();
  auto dest_start_y=dest_start.y();

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
    for (INT64 sj=bsj; sj < bsj+zoom_out && sj < source_start_y+source_copy_h; sj++) {
      if (dj < dest_h_visible) {
        for (INT64 bsi=source_start_x, ri=0;
             bsi < source_start_x+source_copy_w;
             bsi+=zoom_out, ri++) {
          for (INT64 si=bsi; si < bsi+zoom_out && si < source_start_x+source_copy_w && si < source_w; si++) {
            auto source_pixel=sj*source_w+si;
            // work out pixel math more carefully so we don't have to do
            // this check every copy, test with asserts
            if (ri < (source_copy_w/zoom_out) && ri < (dest_w_visible-dest_start_x)) {
              REDUCE_ALL_COPY_EXPRESSION;
            }
          }
        }
      }
    }
    for (INT64 di=dest_start_x, ri=0; di < dest_start_x+(source_copy_w/zoom_out); di++, ri++) {
      if (ri < (source_copy_w/zoom_out) &&
          di < dest_w_visible && dj < dest_h_visible) {
        auto dest_pixel=dj*dest_w+di;
        dest_buffer[dest_pixel]=row_buffer[ri*3] >> block_average_shift;
        dest_buffer[dest_pixel]+=row_buffer[ri*3+1] >> block_average_shift << G_SHIFT;
        dest_buffer[dest_pixel]+=row_buffer[ri*3+2] >> block_average_shift << B_SHIFT;
        dest_buffer[dest_pixel]+=DEFAULT_ALPHA;
      }
    }
  }
}
#endif
