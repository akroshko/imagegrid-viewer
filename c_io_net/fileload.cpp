/**
 * The functions that in load in image files.  This file is very much
 * a preliminary work-in-progress while I work on the rest of program.
 *
 */
// local headers
#include "../common.hpp"
#include "../utility.hpp"
#include "fileload.hpp"
#include "../c_misc/buffer_manip.hpp"
// C++ headers
// #include <exception>
#include <filesystem>
#include <fstream>
// #include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
// C headers
#include <cstring>
// C library headers
#include <png.h>
#include <tiffio.h>

// regex to help find files
std::regex regex_digits_search("([0-9]{1,4})",std::regex_constants::ECMAScript | std::regex_constants::icase);
// match each filetype
// TODO: have better file detection, e.g., magic numbers
// give an empty file descriptor
std::regex empty_search("^\\[\\[EMPTY\\]\\]$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex jpeg_search("\\.jpeg$|\\.jpg$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex png_search("\\.png$",std::regex_constants::ECMAScript | std::regex_constants::icase);
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
  auto found_png=false;
  auto found_jpeg=false;
  auto found_tiff=false;
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
    if (std::regex_search(image_file,png_search)) {
      found_png=true;
      // if actually an extension
      png_files.push_back(image_file);
    } else if (std::regex_search(image_file,jpeg_search)) {
      found_jpeg=true;
      // if actually an extension
      jpeg_files.push_back(image_file);
      // jpeg_files=;
    } else if (std::regex_search(image_file,tiff_search)) {
      found_tiff=true;
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
      if (std::regex_search(png_file,digits_match,regex_digits_search)) {
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
      if (std::regex_search(jpeg_file,digits_match,regex_digits_search)) {
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
      if (std::regex_search(tiff_file,digits_match,regex_digits_search)) {
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
// general file functions

void read_data(std::string filename,
               INT_T &width, INT_T &height) {
  if (check_tiff(filename)) {
    // TODO: check success
    read_tiff_data(filename,
                   width,
                   height);
  } else if (check_png(filename)) {
    // TODO: check success
    read_png_data(filename,
                  width,
                  height);
  } else if (check_empty(filename)) {
    // think that 0 is a reasonable non-image for these values
    // TODO: other values may be good
    width=0;
    height=0;
  } else {
    ERROR("read_data can't read: " << filename);
  }
}

bool load_data_as_rgb(const std::string filename,
                      const std::string cached_filename,
                      const CURRENT_SUBGRID_T current_subgrid,
                      const std::vector<std::shared_ptr<LoadSquareData>> load_file_data) {
  bool load_successful=false;
  if (check_tiff(filename)) {
    MSG("Loading TIFF: " << filename);
    // TODO: check success
    load_tiff_as_rgb(filename,
                     cached_filename,
                     current_subgrid,
                     load_file_data);
    // printing pointer here
    MSG("Done TIFF: " << filename);
    load_successful=true;
  } else if (check_png(filename)) {
      MSG("Loading PNG: " << filename);
      // TODO: check success
      load_png_as_rgb(filename,
                      current_subgrid,
                      load_file_data);
      MSG("Done PNG: " << filename);
      load_successful=true;
  } else if (check_empty(filename)) {
  } else {
    ERROR("load_data_as_rgb can't load: " << filename);
  }
  return load_successful;
}

////////////////////////////////////////////////////////////////////////////////
// load specific files as RGB

bool read_tiff_data(std::string filename,
                    INT_T &width, INT_T &height) {
  auto success=false;

  TIFF* tif=TIFFOpen(filename.c_str(), "r");

  if (!tif) {
    ERROR("load_tiff_as_rgb() Failed to allocate raster for: " << filename);
  } else {
    uint32_t w,h;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    width=w;
    height=h;
    TIFFClose(tif);
    success=true;
  }
  return success;
}

bool load_tiff_as_rgb(const std::string filename,
                      const std::string cached_filename,
                      const CURRENT_SUBGRID_T current_subgrid,
                      const std::vector<std::shared_ptr<LoadSquareData>> load_file_data) {
  auto success=false;
  TIFF* tif=TIFFOpen(filename.c_str(), "r");
  if (!tif) {
    ERROR("load_tiff_as_rgb() Failed to allocate raster for: " << filename);
  } else {
    uint32_t tiff_width,tiff_height;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &tiff_width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &tiff_height);
    // cache this data somewhere...
    // do a trial first to see if we can use the cached file
    // it is ironic this is in PNG...
    auto can_cache=true;
    if (!std::filesystem::exists(cached_filename)) {
      can_cache=false;
      MSG("Cached file does not exist: " << cached_filename);
    }
    if (can_cache) {
      MSG("Cached file exists: " << cached_filename);
      for (auto & file_data : load_file_data) {
        auto zoom_out_value=file_data->zoom_out_value;
        size_t width_reduced=reduce_and_pad(tiff_width,zoom_out_value);
        size_t height_reduced=reduce_and_pad(tiff_height,zoom_out_value);
        MSG("Testing cache");
        MSG("w: " << width_reduced);
        MSG("h: " << height_reduced);
        if (width_reduced > CACHE_MAX_PIXEL_SIZE || height_reduced > CACHE_MAX_PIXEL_SIZE) {
          MSG("cached failed to be useful");
          can_cache=false;
        }
      }
    }
    if (can_cache) {
      MSG("Using cached file: " << cached_filename);
      INT_T cached_zoom_out_value=1;
      auto width_test=tiff_width;
      auto height_test=tiff_height;
      // this can probably be optimized out of a while loop
      while (width_test > CACHE_MAX_PIXEL_SIZE || height_test > CACHE_MAX_PIXEL_SIZE) {
        cached_zoom_out_value*=ZOOM_STEP;
        width_test=reduce_and_pad(tiff_width,cached_zoom_out_value);
        height_test=reduce_and_pad(tiff_height,cached_zoom_out_value);
      }
      png_image png_image_local;
      // TODO: check libpng library, may not want this
      memset(&png_image_local, 0, (sizeof png_image_local));
      png_image_local.version=PNG_IMAGE_VERSION;
      if (png_image_begin_read_from_file(&png_image_local, cached_filename.c_str()) == 0) {
        ERROR("load_tiff_as_rgb() failed to read from png file: " << cached_filename);
        can_cache=false;
      } else {
        TIFFClose(tif);
        png_bytep png_raster;
        png_image_local.format=PNG_FORMAT_RGB;
        png_raster=new unsigned char[PNG_IMAGE_SIZE(png_image_local)];
        if (png_raster == NULL) {
          ERROR("load_tiff_as_rgb() failed to allocate png buffer!");
          can_cache=false;
        } else {
          if (png_image_finish_read(&png_image_local, NULL, png_raster, 0, NULL) == 0) {
            ERROR("load_tiff_as_rgb() failed to read full png image!");
            can_cache=false;
          } else {
            // TODO: test for mismatched size
            auto png_width=(size_t)png_image_local.width;
            auto png_height=(size_t)png_image_local.height;
            for (auto & file_data : load_file_data) {
              auto zoom_out_value=file_data->zoom_out_value;
              auto actual_zoom_out_value=file_data->zoom_out_value/cached_zoom_out_value;
              // TODO: might want to add an assert here but should be safe due to earlier check
              size_t w_reduced=reduce_and_pad(tiff_width,zoom_out_value);
              size_t h_reduced=reduce_and_pad(tiff_height,zoom_out_value);
              file_data->rgb_wpixel[current_subgrid]=w_reduced;
              file_data->rgb_hpixel[current_subgrid]=h_reduced;
              size_t npixel_reduced=w_reduced*h_reduced;
              file_data->rgb_data[current_subgrid]=new unsigned char[npixel_reduced*3];
              buffer_copy_reduce_generic((unsigned char *)png_raster,png_width,png_height,
                                         0, 0,
                                         file_data->rgb_data[current_subgrid],w_reduced,h_reduced,
                                         actual_zoom_out_value);
            }
          }
          delete[] png_raster;
          return success;
        }
      }
    }
    if (!can_cache) {
      size_t npixels;
      uint32_t* raster;
      npixels=tiff_width*tiff_height;
      raster=(uint32_t*) _TIFFmalloc(npixels * sizeof (uint32_t));
      if (raster == NULL) {
        ERROR("Failed to allocate raster for: " << filename);
      } else {
        if (!TIFFReadRGBAImageOriented(tif, tiff_width, tiff_height, raster, ORIENTATION_TOPLEFT, 0)) {
          ERROR("Failed to read: " << filename);
        } else {
          // convert raster
          for (auto & file_data : load_file_data) {
            auto zoom_out_value=file_data->zoom_out_value;
            size_t w_reduced=reduce_and_pad(tiff_width,zoom_out_value);
            size_t h_reduced=reduce_and_pad(tiff_height,zoom_out_value);
            file_data->rgb_wpixel[current_subgrid]=w_reduced;
            file_data->rgb_hpixel[current_subgrid]=h_reduced;
            auto npixels_reduced=w_reduced*h_reduced;
            file_data->rgb_data[current_subgrid]=new unsigned char[npixels_reduced*3];
            buffer_copy_reduce_tiff(raster,tiff_width,tiff_height,
                                    file_data->rgb_data[current_subgrid],w_reduced,h_reduced,
                                    zoom_out_value);
          }
          success=true;
        }
        _TIFFfree(raster);
      }
      TIFFClose(tif);
    }
  }
  return success;
}

bool read_png_data(std::string filename,
                   INT_T &width, INT_T &height) {
  bool success=false;
  png_image image;
  memset(&image, 0, (sizeof image));
  image.version=PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename.c_str()) == 0) {
    ERROR("read_png_data() failed to read from file: " << filename);
  } else {
    width=(size_t)image.width;
    height=(size_t)image.height;
    png_image_free(&image);
    success=true;
  }
  return success;
}

bool load_png_as_rgb(std::string filename,
                     const CURRENT_SUBGRID_T current_subgrid,
                     const std::vector<std::shared_ptr<LoadSquareData>> load_file_data) {
  bool success=false;
  png_image image;

  memset(&image, 0, (sizeof image));
  // TODO: check libpng library, may not want this
  image.version=PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename.c_str()) == 0) {
    ERROR("load_png_as_rgb() failed to read from file: " << filename);
    success=false;
  } else {
    png_bytep raster;
    image.format=PNG_FORMAT_RGB;
    raster=new unsigned char[PNG_IMAGE_SIZE(image)];
    if (raster == NULL) {
      ERROR("load_png_as_rgb() failed to allocate buffer!");
    } else {
      if (png_image_finish_read(&image, NULL, raster, 0, NULL) == 0) {
        ERROR("load_png_as_rgb() failed to read full image!");
      } else {
        // TODO: test for mismatched size
        auto width=(size_t)image.width;
        auto height=(size_t)image.height;
        for (auto & file_data : load_file_data) {
          auto zoom_out_value=file_data->zoom_out_value;
          size_t w_reduced=reduce_and_pad(width,zoom_out_value);
          size_t h_reduced=reduce_and_pad(height,zoom_out_value);
          file_data->rgb_wpixel[current_subgrid]=w_reduced;
          file_data->rgb_hpixel[current_subgrid]=h_reduced;
          size_t npixel_reduced=w_reduced*h_reduced;
          file_data->rgb_data[current_subgrid]=new unsigned char[npixel_reduced*3];
          buffer_copy_reduce_generic((unsigned char *)raster,width,height,
                                     0, 0,
                                     file_data->rgb_data[current_subgrid],w_reduced,h_reduced,
                                     zoom_out_value);
          success=true;
        }
      }
      delete[] raster;
    }
    png_image_free(&image);
  }
  return success;
}

bool write_png(std::string filename_new, INT_T wpixel, INT_T hpixel, unsigned char* rgb_data) {
   char new_filename[PATH_BUFFER_SIZE]="";
   // TODO: catch exception for overly long filenames once exceptions are handled
   auto c_str=filename_new.c_str();
   auto c_size=filename_new.size();
   if (c_size < PATH_BUFFER_SIZE) {
     strncpy(new_filename,c_str,c_size);
   } else {
     throw std::runtime_error("Invalid filename size.");
   }
   // write a PNG
   // TODO: put this elsewhere
   png_image image;
   memset(&image, 0, (sizeof image));
   image.version=PNG_IMAGE_VERSION;
   image.opaque=NULL;
   image.width=wpixel;
   image.height=hpixel;
   image.format=PNG_FORMAT_RGB;
   image.flags=0;
   image.colormap_entries=0;
   png_image_write_to_file(&image, new_filename, 0, (void*)rgb_data, 0, 0);
   return true;
}

bool check_tiff(std::string filename) {
  return std::regex_search(filename,tiff_search);
}

bool check_png(std::string filename) {
  return std::regex_search(filename,png_search);
}

bool check_empty(std::string filename) {
  return std::regex_search(filename,empty_search);
}

void load_image_grid_from_text (std::string text_file,
                                FILE_DATA_T& file_data) {
  // regex to parse text files, this is a fairly rigid for now since
  // it is autogenerated by a Python script but will probably be
  // superceded by XML
  std::regex regex_text_file_line("^([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+) ([^ ]+)$",std::regex_constants::ECMAScript | std::regex_constants::icase);
  // open the text file
  std::ifstream text_fh(text_file);
  // read line by line
  std::string line;
  while (std::getline(text_fh, line)) {
    std::smatch matched_line;
    // ignore nonmatching lines for now
    if (std::regex_search(line,matched_line,regex_text_file_line)) {
      auto grid_x=std::stoi(matched_line.str(1));
      auto grid_y=std::stoi(matched_line.str(2));
      auto subgrid_x=std::stoi(matched_line.str(3));
      auto subgrid_y=std::stoi(matched_line.str(4));
      auto filename=matched_line.str(5);
      // add to our filedata structure
      auto current_grid=std::pair<INT_T,INT_T>(grid_x,grid_y);
      auto current_subgrid=CURRENT_SUBGRID_T(subgrid_x,subgrid_y);
      // updates
      file_data[current_grid][current_subgrid]=filename;
    }
  }
}
