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

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  int opt;
  INT_T wimage, himage;

  // get options
  while((opt=getopt(argc ,argv, "w:h:p:")) != -1) {
    switch(opt) {
    case 'w':
      wimage=atoi(optarg);
      break;
    case 'h':
      himage=atoi(optarg);
      break;
    case 'p':
      strncpy(this->_path_value,optarg,PATH_BUFFER_SIZE);
      break;
    case '?':
      if (optopt == 'w' || optopt == 'h' || optopt == 'p') {
        std::cerr << "Option " << optopt << " requires an argument.";
      } else {
        std::cerr << "Unknown option: " << (char)optopt << std::endl;
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
  // load number images if this is appropriate
  if (this->_path_value[0] != 0) {
    this->_filenames=load_numbered_images(std::string(this->_path_value));
  }
  //
  this->_successful=true;
};
