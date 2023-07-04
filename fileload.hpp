/**
 * Header for the functions to load files in.  These are very
 * preliminary while I work on the rest of the program and will likely
 * be substantially improved.
 */
#ifndef FILELOAD_HPP
#define FILELOAD_HPP

//local includes
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
// C++ headers
#include <utility>
#include <vector>
#include <string>
#include <regex>

enum IMAGEDIRECTION {tl_horiz_reset,tl_horiz_follow};

/**
 * Load numbered images from a path in order
 *
 * @param  images_path the path to load images from
 *
 * TODO: add a direction
 */
std::vector<std::string> load_numbered_images(std::string images_path// , IMAGEDIRECTION direction
                                    );

/**
 * From a set of image file, find a sequential set of images.
 *
 * @param images_files: vector of image files to find a sequence in
 *
 * TODO: This is a very fragile function that tends to favor one type
 *       of file file over another, not sure if this will only be used
 *       for testing or if it's a more permanent way to load images.
 */
std::vector<std::string> find_sequential_images(std::vector<std::string> image_files);

/**
 * Read data about a tiff file using libtiff,based off of
 * http://www.libtiff.org/libtiff.html
 *
 * @param filename the filename to load
 *
 * @param width set as the width of the image in pixels
 *
 * @param height set as the height of the image in pixels
 */
bool read_tiff_data(std::string filename, INT_T &width, INT_T &height);

/**
 * Load a tiff file using libtiff,based off of
 * http://www.libtiff.org/libtiff.html
 *
 * @param filename the filename to load
 *
 * @param width width of the image in pixels, generally used to check
 * discrepencies
 *
 * @param height height of the image in pixels, generally used to
 * check discrepencies
 *
 * @param rgb_data set as the rgb data from the image
 *
 * @param zoom_level a reduction factor for the image
 */
bool load_tiff_as_rgb(std::string filename,
                      size_t &width, size_t &height,
                      unsigned char** rgb_data,
                      INT_T zoom_level);


/**
 * Read data about a png file using libpng.
 *
 * @param filename the filename to load
 *
 * @param width set as the width of the image in pixels
 *
 * @param height set as the height of the image in pixels
 *
 * @param zoom_level a reduction factor for the image
 */
bool read_png_data(std::string filename, INT_T &width, INT_T &height);

/**
 * Load a png file using libpng.
 *
 * @param filename the filename to load
 *
 * @param width width of the image in pixels, generally used to check
 * discrepencies
 *
 *
 @param height height of the image in pixels, generally used to
 * check discrepencies
 *
 * @param rgb_data set as the rgb data from the image
 */
bool load_png_as_rgb(std::string filename,
                     size_t &width, size_t &height,
                     unsigned char** rgb_data,
                     INT_T zoom_level);

/**
 * Check if a file is a tiff file.
 *
 * @param filename the filname to check
 */
bool check_tiff(std::string filename);

/**
 * Check if a file is a png file.
 *
 * @param filename the filname to check
 */
bool check_png(std::string filename);

#endif
