#ifndef FILELOAD_HPP

#define FILELOAD_HPP

//local includes
#include "imagegrid-viewer.hpp"
// C++ headers
#include <utility>
#include <vector>
#include <string>
#include <regex>
using namespace std;

enum IMAGEDIRECTION {tl_horiz_reset,tl_horiz_follow};

// load numbered images from a path in order
//   images_path: the path to load images from
//   TODO: add a direction
vector<string> load_numbered_images(string images_path// , IMAGEDIRECTION direction
                                    );

// from images files find a sequence of images
//   images_files: list of image files to find a sequence in
//   TODO: this is a very fragile function that tends to favor one type of file
//         file over another, not sure if this will be used for testing or if it's
//         a permanent thing
vector<string> find_sequential_images(vector<string> image_files);

// load a tiff file using libtif
// this is based off of http://www.libtiff.org/libtiff.html
//   filename: the filename to load
//   width: set as the width of the image in pixels
//   height: set as the height of the image in pixels
//   rgb_data: set as the rgb data from the image
bool load_tiff_as_rgb(string filename, int &width, int &height, unsigned char** rgb_data);

// load a tiff file using libpng
//   filename: the filename to load
//   width: set as the width of the image in pixels
//   height: set as the height of the image in pixels
//   rgb_data: set as the rgb data from the image
bool load_png_as_rgb(string filename, int &width, int &height, unsigned char** rgb_data);

// check if a file is a tiff file
//   filename: the filname to check
bool check_tiff(string filename);

// check if a file is a png file
//   filename: the filname to check
bool check_png(string filename);

#endif
