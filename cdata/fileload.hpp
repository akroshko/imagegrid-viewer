/**
 * Header for the functions to load files in.  These are very
 * preliminary while I work on the rest of the program and will likely
 * be substantially improved.
 */
#ifndef FILELOAD_HPP
#define FILELOAD_HPP

//local includes
#include "../common.hpp"
// C++ headers
#include <string>
#include <regex>
#include <vector>
#include <utility>
// C headers
#include <climits>

enum IMAGEDIRECTION {tl_horiz_reset,tl_horiz_follow};

/**
 * Load numbered images from a path in order
 *
 * @param images_path The path to load images from.
 *
 * TODO: add a direction
 */
std::vector<std::string> load_numbered_images(std::string images_path// , IMAGEDIRECTION direction
                                    );

/**
 * From a set of image file, find a sequential set of images.
 *
 * @param images_files Vector of image files to find a sequence in.
 *
 * TODO: This is a very fragile function that tends to favor one type
 *       of file file over another, not sure if this will only be used
 *       for testing or if it's a more permanent way to load images.
 */
std::vector<std::string> find_sequential_images(std::vector<std::string> image_files);

/**
 * Contains loaded file data in preparation to be transferred to
 * ImageGridSquareZoomLevel.
 *
 * May not be permanent, members correspond to those in
 * ImageGridSquareZoomLevel.
 */
struct LoadFileData {
  std::string filename;
  unsigned char* rgb_data=nullptr;
  size_t rgb_wpixel=INT_MIN;
  size_t rgb_hpixel=INT_MIN;
  INT_T zoom_out_value=INT_MIN;
};

/**
 * Read data from a filename.
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
void read_data(std::string filename,
               INT_T &width, INT_T &height);

/**
 * Load data as RGB.
 *
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool load_data_as_rgb(const std::string filename,
                      const std::string cached_filename,
                      const std::vector<std::shared_ptr<LoadFileData>> load_file_data);

/**
 * Read data about a tiff file using libtiff,based off of
 * http://www.libtiff.org/libtiff.html
 *
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool read_tiff_data(std::string filename, INT_T &width, INT_T &height);

/**
 * Load a tiff file using libtiff,based off of
 * http://www.libtiff.org/libtiff.html
 *
 * @param filename The filename to load.
 * @param cached_filename The filename that cached the parts of the
 *                        image fitting in 512x512.
 * @param load_file_data A vector structs to be updated with data as
 *                       it is loaded.
 * @return If loading image was successful.
 */
bool load_tiff_as_rgb(const std::string filename,
                      const std::string cached_filename,
                      const std::vector<std::shared_ptr<LoadFileData>> load_file_data);

/**
 * Read data about a png file using libpng.
 *
 * @param filename The filename to load.
 * @param width Set as the width of the image in pixels.
 * @param height Set as the height of the image in pixels.
 * @return If reading image data was successful.
 */
bool read_png_data(std::string filename, INT_T &width, INT_T &height);

/**
 * Load a png file using libpng.
 *
 * @param filename The filename to load.
 * @param load_file_data A vector structs to be updated with data as
 *                       it is loaded.
 * @return If loading image was successful.
 */
bool load_png_as_rgb(std::string filename,
                     const std::vector<std::shared_ptr<LoadFileData>> load_file_data);

/**
 * Write a png file using libpng.
 *
 * @param filename_new The filename to write.
 * @param wpixel The width in pixels.
 * @param hpixel The height in pixels.
 * @param rgb_data The rgb data.
 * @return If writing image was successful.
 */
bool write_png(std::string filename_new, INT_T wpixel, INT_T hpixel, unsigned char* rgb_data);

/**
 * Check if a file is a tiff file.
 *
 * @param filename The filname to check.
 * @return If the file is a tiff file.
 */
bool check_tiff(std::string filename);

/**
 * Check if a file is a png file.
 *
 * @param filename The filname to check.
 * @return If the file is a png file.
 */
bool check_png(std::string filename);

/**
 * Check if a file is an empty file placeholder.
 *
 * @param filename The filname to check.
 * @return If the file is an empty placeholder file.
 */
bool check_empty(std::string filename);

#endif
