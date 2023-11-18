/**
 * Copy without reducing size of an RGBA buffer. A "safe"
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
 * @param dest_w_visible The max width to use in the destination buffer.
 * @param dest_h_visible The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 */
void (NOREDUCE_FUNCNAME) (SOURCE_TYPE source_buffer, INT64 source_w, INT64 source_h,
                          INT64 source_start_x, INT64 source_start_y,
                          INT64 source_copy_w, INT64 source_copy_h,
                          PIXEL_RGBA* dest_buffer,
                          INT64 dest_w, INT64 dest_h,
                          INT64 dest_w_visible, INT64 dest_h_visible,
                          INT64 dest_start_x, INT64 dest_start_y);

/**
 * Copy and reduce size of an RGBA buffer by a factor of 2. A "safe"
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
 * @param dest_w_visible The max width to use in the destination buffer.
 * @param dest_h_visible The max height to use in the destination buffer.
 * @param dest_start_x An x offset for the origin.
 * @param dest_start_y A y offset for the origin.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE2_FUNCNAME) (SOURCE_TYPE source_buffer, INT64 source_w, INT64 source_h,
                         INT64 source_start_x, INT64 source_start_y,
                         INT64 source_copy_w, INT64 source_copy_h,
                         PIXEL_RGBA* dest_buffer,
                         INT64 dest_w, INT64 dest_h,
                         INT64 dest_w_visible, INT64 dest_h_visible,
                         INT64 dest_start_x, INT64 dest_start_y,
                         INT64* const row_buffer);


/**
 * Copy and reduce size of an RGBA buffer. A "safe" version that
 * should work with any zoom_out_shift and on any 64-bit platform.
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
 * @param zoom_out_shift The factor to expand the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE_MAX_8_FUNCNAME) (SOURCE_TYPE source_buffer, INT64 source_w, INT64 source_h,
                              INT64 source_start_x, INT64 source_start_y,
                              INT64 source_copy_w, INT64 source_copy_h,
                              PIXEL_RGBA* dest_buffer,
                              INT64 dest_w, INT64 dest_h,
                              INT64 dest_w_visible, INT64 dest_h_visible,
                              INT64 dest_start_x, INT64 dest_start_y,
                              INT64 zoom_out_shift,
                              INT64* const row_buffer);

/**
 * Copy and reduce size of an RGBA buffer. A "safe" version that
 * should work with any zoom_out_shift and on any 64-bit platform.
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
 * @param zoom_out_shift The factor to expand the image by as a bit shift.
 * @param row_buffer A working buffer of size at least (source_copy_w >> zoom_out_shift)*3).
 */
void (REDUCE_ALL_FUNCNAME) (SOURCE_TYPE source_buffer, INT64 source_w, INT64 source_h,
                            INT64 source_start_x, INT64 source_start_y,
                            INT64 source_copy_w, INT64 source_copy_h,
                            PIXEL_RGBA* dest_buffer,
                            INT64 dest_w, INT64 dest_h,
                            INT64 dest_w_visible, INT64 dest_h_visible,
                            INT64 dest_start_x, INT64 dest_start_y,
                            INT64 zoom_out_shift,
                            INT64* const row_buffer);
