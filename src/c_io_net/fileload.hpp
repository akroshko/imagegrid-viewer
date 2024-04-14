/**
 * Header for the functions to load files in.  These are very
 * preliminary while I work on the rest of the program and will likely
 * be substantially improved.
 */
#ifndef FILELOAD_HPP
#define FILELOAD_HPP

//local includes
#include "../common.hpp"
#include "../datatypes/coordinates.hpp"
// C++ headers
#include <list>
#include <string>
#include <vector>

// don't like these forward declarations
class LoadFileDataTransfer;
class LoadFileZoomLevelData;

enum IMAGEDIRECTION {tl_horiz_reset,tl_horiz_follow};

const std::string TEXT_EXTENSION{"txt"};

const std::string IMAGEGRID_CACHE_DIRECTORY{"__imagegrid__cache__"};

/**
 * Load numbered images from a path in order
 *
 * TODO: add a direction
 *
 * @param images_path The path to load images from.
 */
std::vector<std::string> load_numbered_images(std::string images_path// , IMAGEDIRECTION direction
  );

/**
 * From a set of image file, find a sequential set of images.
 *
 * TODO: This is a very fragile function that tends to favor one type
 *       of file over another, not sure if this will only be used for
 *       testing or if it's a more permanent way to load images.
 *
 * @param images_files Vector of image files to find a sequence in.
 */
std::vector<std::string> find_sequential_images(std::vector<std::string> image_files);

/**
 * Read data from a filename.
 *
 * @param filename The filename to load.
 * @param use_cache Whether to use cache for reading the file data.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool read_data(const std::string& filename,
               bool use_cache,
               INT64& width,
               INT64& height);

/**
 * Load data as RGB.
 *
 * @param filename The filename to load.
 * @param cached_filename The filename that cached the parts of the
 *                        image fitting in 512x512.
 * @param current_subgrid The current subgrid to load.
 * @param data_transfer The object used to transfer loaded data.
 * @param row_temp_buffer A buffer to use as a working area when loading images.
 * @return If reading image data was successful.
 */
bool load_data_as_rgba(const std::string& filename,
                       const std::string& cached_filename,
                       SubGridIndex& current_subgrid,
                       LoadFileDataTransfer& data_transfer,
                       INT64* row_temp_buffer);

/**
 * Read data about a tiff file using libtiff

 * Based off of http://www.libtiff.org/libtiff.html
 *
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool read_tiff_data(const std::string& filename, INT64& width, INT64& height);


/** Test if caching makes sense for this tiff file.
 *
 * @param cached_filename The filename that cached the parts of the
 *                        image fitting in 512x512.
 * @param data_transfer The object used to transfer loaded data.
 * @return If loading image was successful.
 */
bool test_tiff_cache(const std::string& cached_filename,
                     LoadFileDataTransfer& data_transfer);

/**
 * Load a tiff file using cached value.
 *
 * @param cached_filename The filename that cached the parts of the
 *                        image fitting in 512x512.
 * @param current_subgrid The current subgrid to load.
 * @param data_transfer The object used to transfer loaded data.
 * @param row_temp_buffer A buffer to use as a working area when loading images.
 * @return If loading image was successful.
 */
bool load_tiff_as_rgba_cached(const std::string& cached_filename,
                              SubGridIndex& current_subgrid,
                              LoadFileDataTransfer& data_transfer,
                              INT64* row_temp_buffer);

/**
 * Load a tiff file using libtiff.
 *
 * Based off of http://www.libtiff.org/libtiff.html
 *
 * @param filename The filename to load.
 * @param current_subgrid The current subgrid to load.
 * @param data_transfer The object used to transfer loaded data.
 * @param row_temp_buffer A buffer to use as a working area when loading images.
 * @return If loading image was successful.
 */
bool load_tiff_as_rgba(const std::string& filename,
                      SubGridIndex& current_subgrid,
                      LoadFileDataTransfer& data_transfer,
                       INT64* row_temp_buffer);

/**
 * Read data about a png file using libpng.
 *
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool read_png_data(const std::string& filename, INT64& width, INT64& height);

/**
 * Load a png file using libpng.
 *
 * @param filename The filename to load.
 * @param current_subgrid The current subgrid to load.
 * @param data_transfer The object used to transfer loaded data.
 * @param row_temp_buffer A buffer to use as a working area when loading images.
 * @return If loading image was successful.
 */
bool load_png_as_rgba(const std::string& filename,
                      SubGridIndex& current_subgrid,
                      LoadFileDataTransfer& data_transfer,
                      INT64* row_temp_buffer);

/**
 * Write a png file using libpng.
 *
 * @param filename_png The png filename to write.
 * @param filename_text The text filename to write containing data about the image within.
 * @param wpixel The width in pixels.
 * @param hpixel The height in pixels.
 * @param full_wpixel The full width in pixels to write to text file.
 * @param full_hpixel The full height in pixels to write to text file.
 * @param rgb_data The rgb data.
 * @return If writing image was successful.
 */
bool write_png_text(std::string filename_png, std::string filename_text,
                    INT64 wpixel, INT64 hpixel,
                    INT64 full_wpixel, INT64 full_hpixel,
                    PIXEL_RGBA* rgb_data);

/**
 * Check if a file is a tiff file.
 *
 * @param filename The filname to check.
 * @return If the file is a tiff file.
 */
bool check_tiff(const std::string& filename);

/**
 * Check if a file is a png file.
 *
 * @param filename The filname to check.
 * @return If the file is a png file.
 */
bool check_png(const std::string& filename);

/**
 * Check if a file is part of the Canadian National Topographic System
 * (NTS).
 *
 * This just checks for a zip file for now, will obviously have to be
 * expanded.
 *
 * @param filename The filname to check.
 * @return If the file is part of the NTS system.
 */
bool check_nts(const std::string& filename);

/**
 * Check if a file is an empty file placeholder.
 *
 * @param filename The filname to check.
 * @return If the file is an empty placeholder file.
 */
bool check_empty(const std::string& filename);

/**
 * Load image grid from a text file.
 *
 * @param text_file The text file to load from.
 * @param read_data The data read from the file.
 * @param max_i Set to the maximum width index found.
 * @param max_j Set to the maximum height index found.
 * @return If reading was successful.
 */
bool load_image_grid_from_text(std::string text_file,
                               std::list<GridSetupFile>& read_data,
                               INT64& max_i,
                               INT64& max_j);

/**
 * Create the cached filename with png extension from the real
 * filename.
 *
 * @param filename The filename to use to create the cached filename.
 * @param extension The extension to create.
 * @return The cached filename.
 */
std::string create_cache_filename(const std::string& filename,
                                  const std::string& extension);

/**
 * Get a temporary tiff file from the Canadian national topographic
 * system.
 *
 * @param filename The filenam of the NTS zip file.
 * @param temp_filename The filename of the temp tiff file.
 * @param tiff_fd The file descriptor for the temp tiff file.
 * @return If loading was successful.
 */
bool get_tiff_from_nts_file(const std::string& filename,
                            std::string& temp_filename,
                            int& tiff_fd);

#endif
