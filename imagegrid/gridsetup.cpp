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

const std::string HELP_STRING=
  "Usage: imagegrid-viewer [-c|-d] -w WIDTH -h HEIGHT IMAGES...\n"
  "       imagegrid-viewer [-c|-d] -f TEXT_FILE\n"
  "\n"
  "  -c        create cache\n"
  "  -d        use cache\n"
  "\n"
  "  -w        width of grid in images\n"
  "  -h        height of grid in images\n"
  "\n"
  "  -f        text file with each line in the format:\n"
  "            X_INDEX Y_INDEX X_SUBGRID_INDEX Y_SUBGRID_INDEX FILENAME\n";

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
  if (!this->_successful) {
    MSG("Error parsing arguments");
    std::cout << HELP_STRING << std::endl;
    return;
  }
  if (this->_text_filename.length() != 0) {
    INT_T max_i,max_j;
    load_image_grid_from_text(this->_text_filename,this->_file_data,
                              max_i,max_j);
    wimage=max_i+1;
    himage=max_j+1;
    this->_grid_image_size=GridImageSize(wimage,himage);
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
