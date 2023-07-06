/**
 * The functions that in load in image files.  This file is very much
 * a preliminary work-in-progress while I work on the rest of program.
 *
 */
// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "fileload.hpp"
#include "utility.hpp"
// C++ headers
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>
// C headers
#include <stdlib.h>
// library headers
#include <tiffio.h>
#include <png.h>

// regex to help find files
std::regex regex_digits_search("([0-9]{1,4})",std::regex_constants::ECMAScript | std::regex_constants::icase);
// match each filetype
// TODO: have better file detection, e.g., magic numbers
std::regex png_search("\\.png$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex jpeg_search("\\.jpeg$|\\.jpg$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex tiff_search("\\.tiff|\\.tif$",std::regex_constants::ECMAScript | std::regex_constants::icase);

std::vector<std::string> load_numbered_images(std::string images_path
                                              // , IMAGEDIRECTION *direction
  ) {
  std::vector<std::string> found_files;
  std::filesystem::path images_path_obj{images_path};
  // find files in image root
  for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator{images_path_obj}) {
    found_files.push_back(entry.path());
  }
  return find_sequential_images(found_files);
};

std::vector<std::string> find_sequential_images(std::vector<std::string> image_files) {
  auto found_png = false;
  auto found_jpeg = false;
  auto found_tiff = false;
  std::vector<std::string> png_files;
  std::vector<std::string> jpeg_files;
  std::vector<std::string> tiff_files;
  std::vector<std::pair<std::string,int>> png_digit_files;
  std::vector<std::pair<std::string,int>> jpeg_digit_files;
  std::vector<std::pair<std::string,int>> tiff_digit_files;
  std::vector<std::string> png_sorted_files;
  std::vector<std::string> jpeg_sorted_files;
  std::vector<std::string> tiff_sorted_files;

  std::vector<std::string> empty_files;

  // find which types of image files are availible
  for (const std::string &image_file:image_files) {
    if (regex_search(image_file,png_search)) {
      found_png = true;
      // if actually an extension
      png_files.push_back(image_file);
    } else if (regex_search(image_file,jpeg_search)) {
      found_jpeg = true;
      // if actually an extension
      jpeg_files.push_back(image_file);
      // jpeg_files=;
    } else if (regex_search(image_file,tiff_search)) {
      found_tiff = true;
      // if actually an extension
      tiff_files.push_back(image_file);
    }
  }
  // TODO: add an error if two or more types found
  // TODO: deduplicate code
  std::smatch digits_match;
  if (found_png) {
    for (const std::string &png_file:png_files) {
      // find last 1-4 digit number in each file
      if (regex_search(png_file,digits_match,regex_digits_search)) {
        // TODO: convert to int
        png_digit_files.push_back(make_pair(png_file,std::stoi(digits_match.str(1))));
      }
    }
    // do they form a sequence
    // TODO: this can be more advanced
    sort(png_digit_files.begin(),png_digit_files.end(), [](const std::pair<std::string,int> &left, const std::pair<std::string,int> &right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int> &thepair:png_digit_files) {
      png_sorted_files.push_back(thepair.first);
    }
    return png_sorted_files;
  }
  if (found_jpeg) {
    for (const std::string &jpeg_file:jpeg_files) {
      // find last 1-4 digit number in each file
      if (regex_search(jpeg_file,digits_match,regex_digits_search)) {
        jpeg_digit_files.push_back(make_pair(jpeg_file,std::stoi(digits_match.str(1))));
      }
    }
    sort(jpeg_digit_files.begin(),jpeg_digit_files.end(), [](const std::pair<std::string,int> &left, const std::pair<std::string,int> &right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int> &thepair:jpeg_digit_files) {
      jpeg_sorted_files.push_back(thepair.first);
    }
    return jpeg_sorted_files;
  }
  if (found_tiff) {
    for (const std::string &tiff_file:tiff_files) {
      // find last 1-4 digit number in each file
      if (regex_search(tiff_file,digits_match,regex_digits_search)) {
        tiff_digit_files.push_back(make_pair(tiff_file,std::stoi(digits_match.str(1))));
      }
    }
    sort(tiff_digit_files.begin(), tiff_digit_files.end(), [](const std::pair<std::string,int> &left, const std::pair<std::string,int> &right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int> &thepair:jpeg_digit_files) {
      tiff_sorted_files.push_back(thepair.first);
    }
    return tiff_sorted_files;
  }
  return empty_files;
}

////////////////////////////////////////////////////////////////////////////////
// load specific files as RGB

bool read_tiff_data(std::string filename,
                    INT_T &width, INT_T &height) {
  auto success=true;

  TIFF* tif = TIFFOpen(filename.c_str(), "r");

  DEBUG("load_tiff_as_rgb() begin: " << filename);
  if (!tif) {
    ERROR("load_tiff_as_rgb() Failed to allocate raster for: " << filename);
    success=false;
  } else {
    uint32 w,h;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    width=w;
    height=h;
    TIFFClose(tif);
  }
  return success;
}

bool load_tiff_as_rgb(std::string filename,
                      size_t &width, size_t &height,
                      unsigned char** rgb_data,
                      INT_T zoom_level) {
  auto success=true;

  TIFF* tif = TIFFOpen(filename.c_str(), "r");

  DEBUG("load_tiff_as_rgb() begin: " << filename);
  if (!tif) {
    ERROR("load_tiff_as_rgb() Failed to allocate raster for: " << filename);
    success=false;
  } else {
    uint32 w,h;
    size_t npixels;
    uint32* raster;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

    // if (width != w) {
    //   ERROR("Mismatched size of tiff width!");
    //   return false;
    // }
    // if (height != h) {
    //   ERROR("Mismatched size of tiff height!");
    //   return false;
    // }

    // size_t w_pad=pad(w,zoom_level);
    // size_t h_pad=pad(h,zoom_level);
    size_t w_reduced=reduce_and_pad(w,zoom_level);
    size_t h_reduced=reduce_and_pad(h,zoom_level);
    npixels=w*h;
    // size_t npixels_pad=w_pad*h_pad;
    size_t npixels_reduced=w_reduced*h_reduced;
    width=w_reduced;
    height=h_reduced;
    raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
    DEBUG(w_reduced);
    DEBUG(h_reduced);
    if (raster == NULL) {
      ERROR("Failed to allocate raster for: " << filename);
      success=false;
    } else {
      if (!TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 0)) {
        ERROR("Failed to read: " << filename);
        success=false;
      } else {
        // convert raster
        // width=(size_t)w;
        // height=(size_t)h;
        *rgb_data = new unsigned char[npixels_reduced*3];
        // naive copy
        // for (size_t i=0; i < npixels; i++) {
        //   (*rgb_data)[i*3]=(unsigned char)TIFFGetR(raster[i]);
        //   (*rgb_data)[i*3+1]=(unsigned char)TIFFGetG(raster[i]);
        //   (*rgb_data)[i*3+2]=(unsigned char)TIFFGetB(raster[i]);
        // }
        // TODO: THIS WILL HAVE TO BE OPTIMIZED!!!! PROBABLY VERY BAD!!!!
        for (size_t j=0; j < h_reduced; j++) {
          for (size_t i=0; i < w_reduced; i++) {
            auto rgb_pixel=j*w_reduced+i;
            INT_T tiff_sum_0=0;
            INT_T tiff_sum_1=0;
            INT_T tiff_sum_2=0;
            INT_T number_sum=0;
            for (size_t tj=j*zoom_level; tj < (j+1)*zoom_level; tj++) {
              for (size_t ti=i*zoom_level; ti < (i+1)*zoom_level; ti++) {
                auto tiff_pixel=tj*w+ti;
                if ((ti < w) && (tj < h)) {
                  tiff_sum_0+=TIFFGetR(raster[tiff_pixel]);
                  tiff_sum_1+=TIFFGetG(raster[tiff_pixel]);
                  tiff_sum_2+=TIFFGetB(raster[tiff_pixel]);
                  number_sum++;
                }

                // if ((ti < w) && (tj < h)) {
                //   // auto tiff_pixel=((i*zoom_level)*h)+(j*zoom_level);
                //   (*rgb_data)[rgb_pixel*3]=(unsigned char)TIFFGetR(raster[tiff_pixel]);
                //   (*rgb_data)[rgb_pixel*3+1]=(unsigned char)TIFFGetG(raster[tiff_pixel]);
                //   (*rgb_data)[rgb_pixel*3+2]=(unsigned char)TIFFGetB(raster[tiff_pixel]);
                //
                // } else {
                //   // TODO double check if this has to be zeroed
                //   (*rgb_data)[rgb_pixel*3]=(unsigned char)0;
                //   (*rgb_data)[rgb_pixel*3+1]=(unsigned char)0;
                //   (*rgb_data)[rgb_pixel*3+2]=(unsigned char)0;
                // }
              }
            }
            (*rgb_data)[rgb_pixel*3]=(unsigned char)round((FLOAT_T)tiff_sum_0/(FLOAT_T)number_sum);
            (*rgb_data)[rgb_pixel*3+1]=(unsigned char)round((FLOAT_T)tiff_sum_1/(FLOAT_T)number_sum);
            (*rgb_data)[rgb_pixel*3+2]=(unsigned char)round((FLOAT_T)tiff_sum_2/(FLOAT_T)number_sum);
          }
        }
      }
      _TIFFfree(raster);
    }
    TIFFClose(tif);
  }
  DEBUG("load_tiff_as_rgb() end: " << filename);
  return success;
}

bool read_png_data(std::string filename,
                   INT_T &width, INT_T &height) {
  bool success;
  png_image image;
  DEBUG("read_png_data() begin");

  memset(&image, 0, (sizeof image));
  image.version = PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename.c_str()) == 0) {
    ERROR("read_png_data() failed to read from file: " << filename);
    success=false;
  } else {
    width=(size_t)image.width;
    height=(size_t)image.height;
    png_image_free(&image);
  }
  DEBUG("read_png_data() end");
  return success;
}

bool load_png_as_rgb(std::string filename,
                     size_t &width, size_t &height,
                     unsigned char** rgb_data,
                     INT_T zoom_level) {
  bool success;
  png_image image;

  DEBUG("load_png_as_rgb() begin");

  memset(&image, 0, (sizeof image));
  image.version = PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename.c_str()) == 0) {
    ERROR("load_png_as_rgb() failed to read from file: " << filename);
    success=false;
  } else {
    png_bytep buffer;
    image.format = PNG_FORMAT_RGB;
    buffer = new unsigned char[PNG_IMAGE_SIZE(image)];
    if (buffer == NULL) {
      ERROR("load_png_as_rgb() failed to allocate buffer!");
      success=false;
    } else {
      if (png_image_finish_read(&image, NULL, buffer, 0, NULL) == 0) {
        ERROR("load_png_as_rgb() failed to read full image!");
      } else {
        // TODO: test for mismatched size
        width=(size_t)image.width;
        height=(size_t)image.height;
        *rgb_data=new unsigned char[PNG_IMAGE_SIZE(image)];
        memcpy(*rgb_data,buffer,PNG_IMAGE_SIZE(image));
      }
      delete[] buffer;
      buffer=nullptr;
    }
    png_image_free(&image);
  }
  DEBUG("load_png_as_rgb() end");
  return success;
}


bool check_tiff(std::string filename) {
  if (regex_search(filename,tiff_search)) {
    return true;
  } else {
    return false;
  }
}

bool check_png(std::string filename) {
  if (regex_search(filename,png_search)) {
    return true;
  } else {
    return false;
  }
}
