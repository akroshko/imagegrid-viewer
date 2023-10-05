/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "../common.hpp"
#include "gridsetup.hpp"
#include "../coordinates.hpp"
#include "../c_io_net/fileload.hpp"
#include "../c_misc/argument_parse.hpp"
// C++ headers
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

bool GridSetup::successful() const {
  return this->_successful;
}

GridImageSize GridSetup::grid_image_size() const {
  return this->_grid_image_size;
}

const FILE_DATA_T& GridSetup::file_data() const {
  return this->_file_data;
}

bool GridSetup::do_cache() const {
  return this->_do_cache;
}

bool GridSetup::use_cache() const {
  return this->_use_cache;
}

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  INT_T wimage, himage;
  parse_standard_arguments(argc, argv, wimage, himage,
                           this->_do_cache, this->_use_cache, this->_successful,
                           this->_path_value, this->_filenames, this->_text_filename);
  if (this->_text_filename.length() != 0) {
    this->_grid_image_size=GridImageSize(wimage,himage);
    load_image_grid_from_text(this->_text_filename,this->_file_data);
    this->_successful=true;
  } else {
    this->_grid_image_size=GridImageSize(wimage,himage);
    ////////////////////////////////////////////////////////////////////////////////
    // to here
    auto grid_size=himage*wimage;
    if (grid_size != this->_filenames.size()) {
      this->_successful=false;
      ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
      return;
    }
    // load images numbered in directory if this is appropriate
    if (this->_path_value.length() != 0) {
      this->_filenames=load_numbered_images(this->_path_value);
    }
    for (INT_T i=0;i < this->_filenames.size();i++) {
      INT_T x=i%wimage;
      INT_T y=i/wimage;
      auto current_grid=std::pair<INT_T,INT_T>(x,y);
      auto current_subgrid=CURRENT_SUBGRID_T(0,0);
      this->_file_data[current_grid][current_subgrid]=this->_filenames[i];
    }
    this->_successful=true;
  }
};
