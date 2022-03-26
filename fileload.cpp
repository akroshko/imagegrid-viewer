// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "fileload.hpp"
#include "imagegrid-viewer.hpp"
// C++ headers
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

using namespace std;

// regex to help find files
regex regex_digits_search("([0-9]{1,4})",regex_constants::ECMAScript | regex_constants::icase);
// match each filetype
// TODO: have better file detection, e.g., magic numbers
regex png_search("\\.png$",regex_constants::ECMAScript | regex_constants::icase);
regex jpeg_search("\\.jpeg$|\\.jpg$",regex_constants::ECMAScript | regex_constants::icase);
regex tiff_search("\\.tiff|\\.tif$",regex_constants::ECMAScript | regex_constants::icase);

vector<string> load_numbered_images(string images_path// , IMAGEDIRECTION *direction
                                    ) {
  vector<string> found_files;
  filesystem::path images_path_obj{images_path};
  // find files in image root
  for (const filesystem::directory_entry& entry : filesystem::directory_iterator{images_path_obj}) {
    found_files.push_back(entry.path());
  }
  // loop through them
  return find_sequential_images(found_files);
};

vector<string> find_sequential_images(vector<string> image_files) {
  bool found_png = false;
  bool found_jpeg = false;
  bool found_tiff = false;
  vector<string> png_files;
  vector<string> jpeg_files;
  vector<string> tiff_files;
  vector<pair<string,int>> png_digit_files;
  vector<pair<string,int>> jpeg_digit_files;
  vector<pair<string,int>> tiff_digit_files;
  vector<string> png_sorted_files;
  vector<string> jpeg_sorted_files;
  vector<string> tiff_sorted_files;

  vector<string> empty_files;

  // find which types of image files are availible
  for (const string &image_file:image_files) {
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
  smatch digits_match;
  if (found_png) {
    for (const string &png_file:png_files) {
      // find last 1-4 digit number in each file
      if (regex_search(png_file,digits_match,regex_digits_search)) {
        // TODO: convert to int
        png_digit_files.push_back(make_pair(png_file,stoi(digits_match.str(1))));
      }
    }
    // do they form a sequence
    // TODO: this can be more advanced
    sort(png_digit_files.begin(),png_digit_files.end(), [](const pair<string,int> &left, const pair<string,int> &right) {
      return left.second < right.second;
    });
    for (const pair<string,int> &thepair:png_digit_files) {
      png_sorted_files.push_back(thepair.first);
    }
    return png_sorted_files;
  }
  if (found_jpeg) {
    for (const string &jpeg_file:jpeg_files) {
      // find last 1-4 digit number in each file
      if (regex_search(jpeg_file,digits_match,regex_digits_search)) {
        jpeg_digit_files.push_back(make_pair(jpeg_file,stoi(digits_match.str(1))));
      }
    }
    sort(jpeg_digit_files.begin(),jpeg_digit_files.end(), [](const pair<string,int> &left, const pair<string,int> &right) {
      return left.second < right.second;
    });
    for (const pair<string,int> &thepair:jpeg_digit_files) {
      jpeg_sorted_files.push_back(thepair.first);
    }
    return jpeg_sorted_files;
  }
  if (found_tiff) {
    for (const string &tiff_file:tiff_files) {
      // find last 1-4 digit number in each file
      if (regex_search(tiff_file,digits_match,regex_digits_search)) {
        tiff_digit_files.push_back(make_pair(tiff_file,stoi(digits_match.str(1))));
      }
    }
    sort(tiff_digit_files.begin(), tiff_digit_files.end(), [](const pair<string,int> &left, const pair<string,int> &right) {
      return left.second < right.second;
    });
    for (const pair<string,int> &thepair:jpeg_digit_files) {
      tiff_sorted_files.push_back(thepair.first);
    }
    return tiff_sorted_files;
  }
  return empty_files;
}

////////////////////////////////////////////////////////////////////////////////
// load specific files as RGB

bool load_tiff_as_rgb(string filename, int &width, int &height, unsigned char** rgb_data) {
  bool success=true;

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

    npixels=w*h;
    raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
    if (raster == NULL) {
      ERROR("Failed to allocate raster for: " << filename);
      success=false;
    } else {
      if (!TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 0)) {
        ERROR("Failed to read: " << filename);
        success=false;
      } else {
        // convert raster
        width=w;
        height=h;
        *rgb_data = new unsigned char[npixels*3];
        DEBUG("Width: " << width << " Height: " << height);
        // naive copy
        for (size_t i=0; i < npixels; i++) {
          (*rgb_data)[i*3]=(unsigned char)TIFFGetR(raster[i]);
          (*rgb_data)[i*3+1]=(unsigned char)TIFFGetG(raster[i]);
          (*rgb_data)[i*3+2]=(unsigned char)TIFFGetB(raster[i]);
        }
      }
      _TIFFfree(raster);
    }
    TIFFClose(tif);
  }
  DEBUG("load_tiff_as_rgb() end: " << filename);
  return success;
}

bool load_png_as_rgb(string filename, int &width, int &height, unsigned char** rgb_data) {
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
        width=image.width;
        height=image.height;
        *rgb_data=new unsigned char[PNG_IMAGE_SIZE(image)];
        DEBUG("Width: " << width << " Height: " << height);
        memcpy(*rgb_data,buffer,PNG_IMAGE_SIZE(image));
      }
      delete[] buffer;
    }
    png_image_free(&image);
  }
  DEBUG("load_png_as_rgb() end");
  return success;
}


bool check_tiff(string filename) {
  if (regex_search(filename,tiff_search)) {
    return true;
  } else {
    return false;
  }
}

bool check_png(string filename) {
  if (regex_search(filename,png_search)) {
    return true;
  } else {
    return false;
  }
}
