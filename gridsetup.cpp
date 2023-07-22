/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "debug.hpp"
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

bool GridSetup::successful() {
  return this->_successful;
}

GridImageSize GridSetup::grid_image_size() {
  return this->_grid_image_size;
}

std::vector<std::string> GridSetup::filenames() {
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
      strcpy(this->_path_value,optarg);
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
