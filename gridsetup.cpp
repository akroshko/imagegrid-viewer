/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "debug.hpp"
#include "defaults.hpp"
#include "error.hpp"
#include "types.hpp"
#include "gridsetup.hpp"
#include "c_compatible/fileload.hpp"
// C++ headers
#include <string>
#include <vector>
// C headers
#include <getopt.h>
#include <string.h>
#include <cstdlib>

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
  int opt;
  INT_T wimage, himage;
  // get options
  while((opt=getopt(argc ,argv, "w:h:p:cd")) != -1) {
    switch(opt) {
    case 'w':
      // width in images
      wimage=atoi(optarg);
      break;
    case 'h':
      // height in images in images
      himage=atoi(optarg);
      break;
    case 'p':
      // get numbered images from path
      strncpy(this->_path_value,optarg,PATH_BUFFER_SIZE);
      break;
    case 'c':
      // only cache images
      this->_do_cache=true;
      break;
    case 'd':
      // use database
      this->_use_cache=true;
      // future consideration
      // dataset identifier ("dataset" identifier is the path where thinga re cached)
      // strncpy(this->_data_set,optarg,PATH_BUFFER_SIZE);
      break;
    case '?':
      if (optopt == 'w' || optopt == 'h' || optopt == 'p' || optopt == 'd') {
        ERROR("Option " << optopt << " requires an argument.");
      } else {
        ERROR("Unknown option: " << (char)optopt << std::endl);
      }
      this->_successful=false;
      return;
    default:
      this->_successful=false;
      return;
    }
  }
  // TODO: fix a warning about initilized values here
  //       and/or argparse better
  this->_grid_image_size=GridImageSize(wimage,himage);
  // get any files on the end
  if (optind != argc) {
    if (this->_path_value[0] != 0) {
      ERROR("Cannot specify both a path and individual files.");
    } else {
      for (auto i=optind; i < argc; i++) {
        this->_filenames.push_back(std::string(argv[i]));
      }
    }
  }
  auto grid_size=himage*wimage;
  if (grid_size != this->_filenames.size()) {
    this->_successful=false;
    ERROR("Number of filenames " << this->_filenames.size() << " does not match grid size " << grid_size);
    return;
  }
  // load number images if this is appropriate
  if (this->_path_value[0] != 0) {
    this->_filenames=load_numbered_images(std::string(this->_path_value));
  }
  this->_successful=true;
};
