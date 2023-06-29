/**
 * Implementation for classes to setup the grid to be viewed.
 */
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "gridsetup.hpp"
// C++ headers
#include <string>
#include <vector>
// C headers
#include <getopt.h>
#include <string.h>

bool GridSetup::successful() {
  return this->_successful;
}

GridSetupFromCommandLine::GridSetupFromCommandLine(int argc, char* const* argv) {
  int opt;
  INT_T wimage, himage;

  // get options
  while((opt = getopt(argc ,argv, "w:h:p:")) != -1) {
    switch(opt) {
    case 'w':
      wimage=atoi(optarg);
      break;
    case 'h':
      himage=atoi(optarg);
      break;
    case 'p':
      strcpy(this->path_value,optarg);
      break;
    case '?':
      if(optopt == 'w' || optopt == 'h' || optopt == 'p') {
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
  this->grid_image_size=GridImageSize(wimage,himage);

  // get any files on the end
  if(optind != argc) {
    if(this->path_value[0] != 0) {
      ERROR("Cannot specify both a path and individual files.");
    } else {
      for(auto i = optind; i < argc; i++) {
        this->filenames.push_back(std::string(argv[i]));
      }
    }
  }
  this->_successful=true;
};
