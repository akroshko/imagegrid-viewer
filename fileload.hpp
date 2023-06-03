#ifndef FILELOAD_HPP

#define FILELOAD_HPP

//local includes
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "imagegrid-viewer.hpp"
// C++ headers
#include <utility>
#include <vector>
#include <string>
#include <regex>

enum IMAGEDIRECTION {tl_horiz_reset,tl_horiz_follow};

// load numbered images from a path in order
//   images_path: the path to load images from
//   TODO: add a direction
std::vector<std::string> load_numbered_images(std::string images_path// , IMAGEDIRECTION direction
                                    );

// from images files find a sequence of images
//   images_files: list of image files to find a sequence in
//   TODO: this is a very fragile function that tends to favor one type of file
//         file over another, not sure if this will be used for testing or if it's
//         a permanent thing
std::vector<std::string> find_sequential_images(std::vector<std::string> image_files);

// load a tiff file using libtif
// this is based off of http://www.libtiff.org/libtiff.html
//   filename: the filename to load
//   width: set as the width of the image in pixels
//   height: set as the height of the image in pixels
//   rgb_data: set as the rgb data from the image
bool load_tiff_as_rgb(std::string filename, int &width, int &height, unsigned char** rgb_data);

// load a tiff file using libpng
//   filename: the filename to load
//   width: set as the width of the image in pixels
//   height: set as the height of the image in pixels
//   rgb_data: set as the rgb data from the image
bool load_png_as_rgb(std::string filename, int &width, int &height, unsigned char** rgb_data);

// check if a file is a tiff file
//   filename: the filname to check
bool check_tiff(std::string filename);

// check if a file is a png file
//   filename: the filname to check
bool check_png(std::string filename);

#endif
