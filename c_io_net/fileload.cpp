/**
 * The functions that in load in image files.  This file is very much
 * a preliminary work-in-progress while I work on the rest of program.
 */
// local headers
#include "../common.hpp"
#include "../utility.hpp"
#include "fileload.hpp"
#include "../c_misc/buffer_manip.hpp"
#include "../coordinates.hpp"
// don't really like this here, but it is here for now
#include "../imagegrid/imagegrid_load_file_data.hpp"
// C++ headers
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
// #include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>
// C headers
#include <cmath>
#include <cstring>
#include <cstddef>
#include <cstdint>
// C library headers
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <tiff.h>
#include <tiffio.h>
#include <unistd.h>
#include <zip.h>

// regex to help find files
std::regex regex_digits_search("([0-9]{1,4})",std::regex_constants::ECMAScript | std::regex_constants::icase);
// match each filetype
// TODO: have better file detection, e.g., magic numbers
// give an empty file descriptor
std::regex empty_search("^\\[\\[EMPTY\\]\\]$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex jpeg_search("\\.jpeg$|\\.jpg$",std::regex_constants::ECMAScript | std::regex_constants::icase);
std::regex nts_search("\\.zip",std::regex_constants::ECMAScript | std::regex_constants::icase);
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
  for (const std::string& image_file:image_files) {
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
    for (const std::string& png_file:png_files) {
      // find last 1-4 digit number in each file
      if (std::regex_search(png_file,digits_match,regex_digits_search)) {
        // TODO: convert to int
        png_digit_files.push_back(make_pair(png_file,std::stoi(digits_match.str(1))));
      }
    }
    // do they form a sequence
    // TODO: this can be more advanced
    sort(png_digit_files.begin(),png_digit_files.end(), [](const std::pair<std::string,int>& left, const std::pair<std::string,int>& right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int>& thepair:png_digit_files) {
      png_sorted_files.push_back(thepair.first);
    }
    return png_sorted_files;
  }
  if (found_jpeg) {
    for (const std::string& jpeg_file:jpeg_files) {
      // find last 1-4 digit number in each file
      if (std::regex_search(jpeg_file,digits_match,regex_digits_search)) {
        jpeg_digit_files.push_back(make_pair(jpeg_file,std::stoi(digits_match.str(1))));
      }
    }
    sort(jpeg_digit_files.begin(),jpeg_digit_files.end(), [](const std::pair<std::string,int>& left, const std::pair<std::string,int>& right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int>& thepair:jpeg_digit_files) {
      jpeg_sorted_files.push_back(thepair.first);
    }
    return jpeg_sorted_files;
  }
  if (found_tiff) {
    for (const std::string& tiff_file:tiff_files) {
      // find last 1-4 digit number in each file
      if (std::regex_search(tiff_file,digits_match,regex_digits_search)) {
        tiff_digit_files.push_back(make_pair(tiff_file,std::stoi(digits_match.str(1))));
      }
    }
    sort(tiff_digit_files.begin(), tiff_digit_files.end(), [](const std::pair<std::string,int>& left, const std::pair<std::string,int>& right) {
      return left.second < right.second;
    });
    for (const std::pair<std::string,int>& thepair:jpeg_digit_files) {
      tiff_sorted_files.push_back(thepair.first);
    }
    return tiff_sorted_files;
  }
  return empty_files;
}

////////////////////////////////////////////////////////////////////////////////
// general file functions

bool read_data(const std::string& filename,
               bool use_cache,
               INT64& width, INT64& height) {
  MSG("Reading: " << filename);
  auto cache_successful=false;
  auto successful=true;
  if (use_cache) {
    // TODO: assuming read always successful if file opens
    //       files are two lines for now
    auto filename_txt=create_cache_filename(filename,TEXT_EXTENSION);
    std::ifstream file_in_text(filename_txt);
    if (std::filesystem::exists(filename_txt)) {
      MSG("Using file: " << filename_txt << " as cache for: " << filename);
      std::string line;
      std::getline(file_in_text,line);
      width=std::stoi(line);
      std::getline(file_in_text,line);
      height=std::stoi(line);
      cache_successful=true;
      successful=true;
    }
  }
  if (!cache_successful) {
    if (check_tiff(filename)) {
      successful=read_tiff_data(filename,
                                width,
                                height);
    } else if (check_png(filename)) {
      successful=read_png_data(filename,
                               width,
                               height);
    } else if (check_nts(filename)) {
      // get temporary tiff
      std::string temp_filename;
      int tiff_fd=-1;
      successful=get_tiff_from_nts_file(filename,
                                        temp_filename,
                                        tiff_fd);
      if (successful) {
        successful=read_tiff_data(temp_filename,
                                  width,
                                  height);
        if (tiff_fd >= 0) {
          close(tiff_fd);
          MSG("Unlinking: " << temp_filename);
          unlink(temp_filename.c_str());
        }
      }
    } else if (check_empty(filename)) {
      // TODO: find way to not use these as sentinel values
      width=0;
      height=0;
    } else {
      ERROR("read_data can't read: " << filename);
    }
  }
  return successful;
}

bool load_data_as_rgba(const std::string& filename,
                       const std::string& cached_filename,
                       SubGridIndex& current_subgrid,
                       LoadFileDataTransfer& data_transfer,
                       INT64* row_temp_buffer) {
  // TODO: update error checking and propogation, not loading a known
  //       filename because of an error is serious but should not
  //       cause program crash, whereas this is also where the check
  //       for empty squares take place
  bool load_successful=false;
  if (check_tiff(filename)) {
    MSG("Loading TIFF: " << filename);
    load_successful=load_tiff_as_rgba_cached(cached_filename,
                                             current_subgrid,
                                             data_transfer,
                                             row_temp_buffer);
    if (!load_successful) {
      load_successful=load_tiff_as_rgba(filename,
                                        current_subgrid,
                                        data_transfer,
                                        row_temp_buffer);
    }
    MSG("Done TIFF: " << filename);
  } else if (check_png(filename)) {
      MSG("Loading PNG: " << filename);
      // TODO: check success
      load_successful=load_png_as_rgba(filename,
                                       cached_filename,
                                       current_subgrid,
                                       data_transfer,
                                       row_temp_buffer);
      MSG("Done PNG: " << filename);
  } else if (check_nts(filename)) {
    // get temporary tiff
    std::string temp_filename;
    int tiff_fd=-1;
    MSG("Loading NTS: " << filename);
    load_successful=load_tiff_as_rgba_cached(cached_filename,
                                             current_subgrid,
                                             data_transfer,
                                             row_temp_buffer);
    if (!load_successful) {
       load_successful=get_tiff_from_nts_file(filename,
                                              temp_filename,
                                              tiff_fd);
       if (load_successful) {
         MSG("Loading TIFF: " << temp_filename);
         load_successful=load_tiff_as_rgba(temp_filename,
                                           current_subgrid,
                                           data_transfer,
                                           row_temp_buffer);
         if (tiff_fd >= 0) {
           close(tiff_fd);
           MSG("Unlinking: " << temp_filename);
           unlink(temp_filename.c_str());
         }
       }
    }
    MSG("Done NTS: " << filename);
  } else if (check_empty(filename)) {
  } else {
    ERROR("load_data_as_rgba can't load: " << filename);
  }
  return load_successful;
}

////////////////////////////////////////////////////////////////////////////////
// load specific files as RGB
bool read_tiff_data(const std::string& filename,
                    INT64& width, INT64& height) {
  auto success=false;

  TIFF* tif=TIFFOpen(filename.c_str(), "r");

  if (!tif) {
    ERROR("load_tiff_as_rgba() Failed to allocate raster for: " << filename);
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

bool test_tiff_cache(const std::string& cached_filename,
                     LoadFileDataTransfer& data_transfer) {
  auto can_cache=true;
  if (check_valid_filename(cached_filename) && !std::filesystem::exists(cached_filename)) {
    can_cache=false;
    MSG("Cached file does not exist: " << cached_filename);
  }
  auto sub_w=data_transfer.sub_w;
  auto sub_h=data_transfer.sub_h;
  if (can_cache) {
    MSG("Cached file exists: " << cached_filename);
    for (const auto& file_data : data_transfer.data_transfer) {
      auto max_wpixel=sub_w*file_data->max_sub_wpixel;
      auto max_hpixel=sub_h*file_data->max_sub_hpixel;
      if (max_wpixel >= CACHE_MAX_PIXEL_SIZE || max_hpixel >= CACHE_MAX_PIXEL_SIZE) {
        MSG("cached failed to be useful");
        can_cache=false;
        break;
      }
    }
  }
  return can_cache;
}

bool load_tiff_as_rgba_cached(const std::string& cached_filename,
                              SubGridIndex& current_subgrid,
                              LoadFileDataTransfer& data_transfer,
                              INT64* row_temp_buffer) {
  auto sub_i=current_subgrid.subgrid_i();
  auto sub_j=current_subgrid.subgrid_j();
  auto successful=false;
  auto sub_w=data_transfer.sub_w;
  auto sub_h=data_transfer.sub_h;
  auto sub_index=sub_j*sub_w+sub_i;
  auto original_width=data_transfer.original_rgba_wpixel[sub_index];
  auto original_height=data_transfer.original_rgba_hpixel[sub_index];
  auto can_cache=test_tiff_cache(cached_filename,
                                 data_transfer);
  if (can_cache) {
    MSG("Using cached file: " << cached_filename);
    INT64 cached_zoom_out=1;
    // TODO: this could be a problem amongst wildly varying image sizes
    //       solutions are to:
    //           store max width/max height vs zoom_out
    //           calculate the value in different places with similar functions
    auto width_test=original_width;
    auto height_test=original_height;
    // TODO: this duplicates the calculation in
    // ImageGrid::_write_cache, there should be a better way once
    // the data structures are revised
    while (sub_w*width_test >= CACHE_MAX_PIXEL_SIZE || sub_h*height_test >= CACHE_MAX_PIXEL_SIZE) {
      cached_zoom_out*=ZOOM_STEP;
      width_test=reduce_and_pad(original_width,cached_zoom_out);
      height_test=reduce_and_pad(original_height,cached_zoom_out);
    }
    png_image png_image_local;
    memset(&png_image_local, 0, sizeof(png_image_local));
    png_image_local.version=PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&png_image_local, cached_filename.c_str()) == 0) {
      ERROR("load_tiff_as_rgba() failed to read from png file: " << cached_filename);
    } else {
      png_bytep png_raster;
      png_image_local.format=PNG_FORMAT_RGBA;
      png_raster=new unsigned char[PNG_IMAGE_SIZE(png_image_local)];
      if (png_raster == NULL) {
        ERROR("load_tiff_as_rgba() failed to allocate png buffer!");
      } else {
        if (png_image_finish_read(&png_image_local, NULL, png_raster, 0, NULL) == 0) {
          ERROR("load_tiff_as_rgba() failed to read full png image!");
        } else {
          // TODO: test for mismatched size
          auto png_width=(size_t)png_image_local.width;
          auto png_height=(size_t)png_image_local.height;
          for (const auto& file_data : data_transfer.data_transfer) {
            auto zoom_out=file_data->zoom_out;
            auto actual_zoom_out=file_data->zoom_out/cached_zoom_out;
            // TODO: might want to add an assert here but should be safe due to earlier check
            size_t w_reduced=reduce_and_pad(original_width,zoom_out);
            size_t h_reduced=reduce_and_pad(original_height,zoom_out);
            auto sub_index_arr=sub_j*sub_w+sub_i;
            file_data->rgba_wpixel[sub_index_arr]=w_reduced;
            file_data->rgba_hpixel[sub_index_arr]=h_reduced;
            size_t npixel_reduced=w_reduced*h_reduced;
            file_data->rgba_data[sub_index_arr]=new PIXEL_RGBA[npixel_reduced];
            buffer_copy_reduce_generic((PIXEL_RGBA *)png_raster,png_width,png_height,
                                       0,0,
                                       png_width,png_height,
                                       file_data->rgba_data[sub_index_arr],
                                       w_reduced,h_reduced,
                                       w_reduced,h_reduced,
                                       0,0,
                                       // TODO: get rid of this float
                                       (INT64)floor(log2(actual_zoom_out)),
                                       row_temp_buffer);
          }
        }
        delete[] png_raster;
        successful=true;
        return successful;
      }
    }
  }
  return successful;
}

bool load_tiff_as_rgba(const std::string& filename,
                       SubGridIndex& current_subgrid,
                       LoadFileDataTransfer& data_transfer,
                       INT64* row_temp_buffer) {
  auto sub_i=current_subgrid.subgrid_i();
  auto sub_j=current_subgrid.subgrid_j();
  auto sub_w=data_transfer.sub_w;
  auto success=false;
  TIFF* tif=TIFFOpen(filename.c_str(), "r");
  if (!tif) {
    ERROR("load_tiff_as_rgba() Failed to allocate raster for: " << filename);
  } else {
    uint32_t tiff_width,tiff_height;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &tiff_width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &tiff_height);
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
        for (const auto& file_data : data_transfer.data_transfer) {
          auto zoom_out=file_data->zoom_out;
          size_t w_reduced=reduce_and_pad(tiff_width,zoom_out);
          size_t h_reduced=reduce_and_pad(tiff_height,zoom_out);
          auto sub_index_arr=sub_j*sub_w+sub_i;
          file_data->rgba_wpixel[sub_index_arr]=w_reduced;
          file_data->rgba_hpixel[sub_index_arr]=h_reduced;
          auto npixels_reduced=w_reduced*h_reduced;
          file_data->rgba_data[sub_index_arr]=new PIXEL_RGBA[npixels_reduced];
          buffer_copy_reduce_tiff(raster,tiff_width,tiff_height,
                                  file_data->rgba_data[sub_index_arr],w_reduced,h_reduced,
                                  // TODO: get rid of this floating point
                                  floor(log2(zoom_out)),
                                  row_temp_buffer);
        }
        success=true;
      }
      _TIFFfree(raster);
    }
    TIFFClose(tif);
  }
  return success;
}

bool read_png_data(const std::string& filename,
                   INT64& width, INT64& height) {
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

bool load_png_as_rgba(const std::string& filename,
                      const std::string& cached_filename,
                      SubGridIndex& current_subgrid,
                      LoadFileDataTransfer& data_transfer,
                      INT64* row_temp_buffer) {
  auto sub_i=current_subgrid.subgrid_i();
  auto sub_j=current_subgrid.subgrid_j();
  auto sub_w=data_transfer.sub_w;
  bool success=false;
  png_image image;
  memset(&image, 0, (sizeof image));
  // TODO: check libpng library, may not want this
  image.version=PNG_IMAGE_VERSION;
  if (png_image_begin_read_from_file(&image, filename.c_str()) == 0) {
    ERROR("load_png_as_rgba() failed to read from file: " << filename);
    success=false;
  } else {
    png_bytep raster;
    image.format=PNG_FORMAT_RGBA;
    raster=new unsigned char[PNG_IMAGE_SIZE(image)];
    if (raster == NULL) {
      ERROR("load_png_as_rgba() failed to allocate buffer!");
    } else {
      if (png_image_finish_read(&image, NULL, raster, 0, NULL) == 0) {
        ERROR("load_png_as_rgba() failed to read full image!");
      } else {
        // TODO: test for mismatched size
        auto width=(size_t)image.width;
        auto height=(size_t)image.height;
        for (const auto& file_data : data_transfer.data_transfer) {
          auto zoom_out=file_data->zoom_out;
          size_t w_reduced=reduce_and_pad(width,zoom_out);
          size_t h_reduced=reduce_and_pad(height,zoom_out);
          auto sub_index_arr=sub_j*sub_w+sub_i;
          file_data->rgba_wpixel[sub_index_arr]=w_reduced;
          file_data->rgba_hpixel[sub_index_arr]=h_reduced;
          size_t npixel_reduced=w_reduced*h_reduced;
          file_data->rgba_data[sub_index_arr]=new PIXEL_RGBA[npixel_reduced];
          buffer_copy_reduce_generic((PIXEL_RGBA*)raster,width,height,
                                     0,0,
                                     width,height,
                                     file_data->rgba_data[sub_index_arr],
                                     w_reduced,h_reduced,
                                     w_reduced,h_reduced,
                                     0,0,
                                     // TODO: get rid of this float
                                     (INT64)floor(log2(zoom_out)),
                                     row_temp_buffer);
          success=true;
        }
      }
      delete[] raster;
    }
    png_image_free(&image);
  }
  return success;
}

bool write_png_text(std::string filename_png,
                    std::string filename_text,
                    INT64 wpixel, INT64 hpixel,
                    INT64 full_wpixel, INT64 full_hpixel,
                    PIXEL_RGBA* rgba_data) {
   char new_filename[PATH_BUFFER_SIZE]="";

   auto c_str=filename_png.c_str();
   auto c_size=filename_png.size();
   if (c_size < PATH_BUFFER_SIZE) {
     strncpy(new_filename,c_str,c_size);
   } else {
     // TODO: make sure this is handled
     return false;
   }
   // write a PNG
   // TODO: encapsulate this writing elsewhere
   png_image image;
   memset(&image, 0, (sizeof image));
   image.version=PNG_IMAGE_VERSION;
   image.opaque=NULL;
   image.width=wpixel;
   image.height=hpixel;
   image.format=PNG_FORMAT_RGBA;
   image.flags=0;
   image.colormap_entries=0;
   png_image_write_to_file(&image, new_filename, 0, (void*)rgba_data, 0, 0);
   // finally write out a text file with vital information
   std::ofstream file_out_text;
   file_out_text.open(filename_text,std::ios::trunc);
   file_out_text << full_wpixel << std::endl << full_hpixel << std::endl;
   file_out_text.close();
   // TODO: add error checking
   return true;
}

bool check_tiff(const std::string& filename) {
  return std::regex_search(filename,tiff_search);
}

bool check_png(const std::string& filename) {
  return std::regex_search(filename,png_search);
}

bool check_nts(const std::string& filename) {
  // TODO: this is going to have to be a more complicated search
  return std::regex_search(filename,nts_search);
}

bool check_empty(const std::string& filename) {
  return std::regex_search(filename,empty_search);
}

void load_image_grid_from_text (std::string text_file,
                                std::list<GridSetupFile>& read_data,
                                INT64& max_i,
                                INT64& max_j) {
  // regex to parse text files, this is a fairly rigid for now since
  // it is autogenerated by a Python script but will probably be
  // superceded by XML
  max_i=0;
  max_j=0;
  std::regex regex_text_file_line("^([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+) ([^ ]+)$",
                                  std::regex_constants::ECMAScript | std::regex_constants::icase);
  // open the text file
  std::ifstream text_fh(text_file);
  // read line by line
  std::string line;
  while (std::getline(text_fh, line)) {
    std::smatch matched_line;
    // ignore nonmatching lines for now
    if (std::regex_search(line,matched_line,regex_text_file_line)) {
      auto grid_i=std::stoi(matched_line.str(1));
      auto grid_j=std::stoi(matched_line.str(2));
      auto subgrid_i=std::stoi(matched_line.str(3));
      auto subgrid_j=std::stoi(matched_line.str(4));
      auto filename=matched_line.str(5);
      // add to new data structure
      GridSetupFile data_entry;
      data_entry.grid_i=grid_i;
      data_entry.grid_j=grid_j;
      data_entry.subgrid_i=subgrid_i;
      data_entry.subgrid_j=subgrid_j;
      data_entry.filename=filename;
      read_data.push_back(data_entry);
      if (grid_i > max_i) {
        max_i=grid_i;
      }
      if (grid_j > max_j) {
        max_j=grid_j;
      }
    }
  }
}

std::string create_cache_filename(const std::string& filename, const std::string& extension) {
  // TODO: this really needs to be both profiled and called less
  std::filesystem::path filename_path{filename};
  auto filename_parent=filename_path.parent_path();
  auto filename_base=filename_path.filename();
  auto filename_ext=filename_path.extension();
  auto filename_stem=filename_base.stem();
  auto filename_new=filename_parent;
  // add extension to beginning to distinguish among otherwise
  // identically named files
  filename_stem=std::filesystem::path(filename_ext.string().substr(1) + "_" + filename_stem.string() + "." + extension);
  filename_new/="__imagegrid__cache__";
  std::filesystem::create_directories(filename_new);
  filename_new/=filename_stem;
  return filename_new.string();
}

////////////////////////////////////////////////////////////////////////////////
// more complex filetypes

bool get_tiff_from_nts_file(const std::string& filename,
                            std::string& temp_filename,
                            int& tiff_fd) {
  // this flips to true when an appropriate file is fond
  bool found=false;
  // this flips to false on error
  bool success=true;
  int zip_error;
  zip_t* zip_struct=NULL;
  const char* zip_name=filename.c_str();
  char zip_internal_name[PATH_BUFFER_SIZE];
  char zip_tiff_name[PATH_BUFFER_SIZE];
  char tiff_temp_filename[PATH_BUFFER_SIZE];
  // TODO: I need to find the best way to get a temp path
  strncpy(tiff_temp_filename,TEMP_TEMPLATE_TIF,PATH_BUFFER_SIZE);
  if (!(zip_struct=zip_open(zip_name,ZIP_RDONLY,&zip_error))) {
    zip_error_t error;
    zip_error_init_with_code(&error, zip_error);
    ERROR("zip_open error: '%s': %s\n" << zip_name << zip_error_strerror(&error));
    zip_error_fini(&error);
    success=false;
  }
  zip_int64_t num_entries;
  if ((num_entries=zip_get_num_entries(zip_struct,0)) < 0) {
    ERROR("No entries in: " << filename);
    success=false;
  }
  // TODO: currently only two entries are available but search could lean a little more heavily on libzip functions
  //       also assuming only one tiff file for now
  //       handling more than one will need a way to identify "correct" tif file
  if (success) {
    for (int i=0; i < num_entries; i++) {
      const char* zip_internal_name_temp;
      zip_internal_name_temp=zip_get_name(zip_struct,i,0);
      if (zip_internal_name_temp) {
        strncpy(zip_internal_name,zip_internal_name_temp,PATH_BUFFER_SIZE);
        int zip_internal_name_length=strnlen(zip_internal_name,PATH_BUFFER_SIZE);
        const char* suffix=NTS_TIF_INTERNAL_EXTENSION;
        const unsigned int suffix_length=strlen(suffix);
        char* suffix_substring=strstr(zip_internal_name,suffix);
        if (suffix_substring &&
            strnlen(suffix_substring,
                    PATH_BUFFER_SIZE-zip_internal_name_length+suffix_length)
            == suffix_length) {
          found=true;
          strncpy(zip_tiff_name,zip_internal_name,PATH_BUFFER_SIZE);
          zip_stat_t tiff_stat;
          zip_stat_init(&tiff_stat);
          if (zip_stat(zip_struct,zip_internal_name,0,&tiff_stat) < 0) {
            ERROR("zip_stat error: " << filename);
            success=false;
          } else {
            zip_uint64_t tiff_size;
            void* tiff_buff=NULL;
            // get size of tiff
            tiff_size=tiff_stat.size;
            if ((tiff_buff=malloc(tiff_size))) {
              zip_file* zip_file_struct;
              if (!(zip_file_struct=zip_fopen(zip_struct,zip_internal_name,0))) {
                ERROR("zip_fopen: " << filename);
                free(tiff_buff);
                success=false;
              } else {
                // TODO: check number of bytes actually read
                if (zip_fread(zip_file_struct,tiff_buff,tiff_size) < 0) {
                  ERROR("zip_fread: " << filename);
                  free(tiff_buff);
                  zip_fclose(zip_file_struct);
                  success=false;
                } else {
                  if (zip_fclose(zip_file_struct) < 0) {
                    ERROR("zip_fclose: " << filename);
                    free(tiff_buff);
                    success=false;
                  } else {
                    // now write the tiff data to a temp file
                    // this file stays open beyond the life of this function if successful
                    if ((tiff_fd=mkstemps(tiff_temp_filename,4)) < 0) {
                      ERROR("tiff_fd: " << filename);
                      free(tiff_buff);
                      success=false;
                    } else {
                      if (write(tiff_fd,tiff_buff,tiff_size) < 0) {
                        ERROR("write: " << filename);
                        free(tiff_buff);
                        close(tiff_fd);
                        tiff_fd=-1;
                        success=false;
                      };
                      if (tiff_buff) { free(tiff_buff); }
                      if (lseek(tiff_fd,0,SEEK_SET) < 0) {
                        ERROR("lseek: " << filename);
                        close(tiff_fd);
                        tiff_fd=-1;
                        success=false;
                      }
                    }
                  }
                }
              }
            } else {
              ERROR("malloc: " << filename);
              success=false;
            }
          }
        }
      }
    }
  }
  temp_filename=std::string(tiff_temp_filename);
  zip_close(zip_struct);
  return found && success;
}
