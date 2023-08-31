/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "debug.hpp"
#include "defaults.hpp"
#include "error.hpp"
#include "types.hpp"
#include "gridsetup.hpp"
#include "c_compatible/fileload.hpp"
#include "c_compatible/argument_parse.hpp"
// C++ headers
#include <string>
#include <vector>

bool GridSetup::successful() const {
  return this->_successful;
}

GridImageSize GridSetup::grid_image_size() const {
  return this->_grid_image_size;
}

std::vector<std::string> GridSetup::filenames() const {
  return this->_filenames;
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
                           this->_path_value, this->_filenames);
  // TODO: fix a warning about initilized values here
  //       and/or argparse better
  this->_grid_image_size=GridImageSize(wimage,himage);
  ////////////////////////////////////////////////////////////////////////////////
  // to here
  auto grid_size=himage*wimage;
  if (grid_size != this->_filenames.size()) {
    this->_successful=false;
    ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
    return;
  }
  // load number images if this is appropriate
  if (this->_path_value.length() != 0) {
    this->_filenames=load_numbered_images(this->_path_value);
  }
  this->_successful=true;
};
