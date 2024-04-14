/**
 * Copy without reducing size of an RGBA buffer. A "safe"
 * version that should work on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 */
void (NOREDUCE_FUNCNAME) (SOURCE_TYPE source_buffer,
                          const BufferPixelSize& source_size,
                          const BufferPixelCoordinate& source_start,
                          const BufferPixelSize& source_copy_size,
                          PIXEL_RGBA* dest_buffer,
                          const BufferPixelSize& dest_size,
                          const BufferPixelSize& dest_size_visible,
                          const BufferPixelCoordinate& dest_start);

/**
 * Copy and reduce size of an RGBA buffer by a factor of 2. A "safe"
 * version that should work on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE2_FUNCNAME) (SOURCE_TYPE source_buffer,
                         const BufferPixelSize& source_size,
                         const BufferPixelCoordinate& source_start,
                         const BufferPixelSize& source_copy_size,
                         PIXEL_RGBA* dest_buffer,
                         const BufferPixelSize& dest_size,
                         const BufferPixelSize& dest_size_visible,
                         const BufferPixelCoordinate& dest_start,
                         INT64* const row_buffer);

/**
 * Copy and reduce size of an RGBA buffer. A "safe" version that
 * should work with any zoom_out_shift and on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 * @param zoom_out_shift The factor to expand the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE_MAX_8_FUNCNAME) (SOURCE_TYPE source_buffer,
                              const BufferPixelSize& source_size,
                              const BufferPixelCoordinate& source_start,
                              const BufferPixelSize& source_copy_size,
                              PIXEL_RGBA* dest_buffer,
                              const BufferPixelSize& dest_size,
                              const BufferPixelSize& dest_size_visible,
                              const BufferPixelCoordinate& dest_start,
                              INT64 zoom_out_shift,
                              INT64* const row_buffer);

/**
 * Copy and reduce size of an RGBA buffer. A "safe" version that
 * should work with any zoom_out_shift and on any 64-bit platform.
 *
 * @param source_buffer The source buffer.
 * @param source_size The size of the source buffer.
 * @param source_start The location on the source buffer to start copying.
 * @param source_copy_size The size of the source buffer to copy.
 * @param dest_buffer The destination buffer.
 * @param dest_size The size of the destination buffer.
 * @param dest_size_visible The visible size of the destination buffer.
 * @param dest_start Where to start copying to on the destination buffer.
 * @param zoom_out_shift The factor to expand the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE_ALL_FUNCNAME) (SOURCE_TYPE source_buffer,
                            const BufferPixelSize& source_size,
                            const BufferPixelCoordinate& source_start,
                            const BufferPixelSize& source_copy_size,
                            PIXEL_RGBA* dest_buffer,
                            const BufferPixelSize& dest_size,
                            const BufferPixelSize& dest_size_visible,
                            const BufferPixelCoordinate& dest_start,
                            INT64 zoom_out_shift,
                            INT64* const row_buffer);
