#include "../common.hpp"
// C++ headers
#include <ostream>
#include <string>
#include <vector>
// C headers
#include <cstdlib>
// C library headers
#include <getopt.h>

void parse_standard_arguments(int argc, char* const* argv,
                              INT_T& wimage, INT_T& himage,
                              bool& do_cache, bool& use_cache, bool& successful,
                              std::string& path_value, std::vector<std::string>& filenames,
                              std::string& text_filename) {
  int opt;
  char *end;
  bool size_arg=false;
  bool file_arg=false;
  // char path_value_local[PATH_BUFFER_SIZE]={ 0 };
  // get options
  while((opt=getopt(argc ,argv, "w:h:p:f:cd")) != -1) {
    switch(opt) {
    case 'w':
      // width in images
      wimage=strtol(optarg,&end,10);
      size_arg=true;
      break;
    case 'h':
      // height in images in images
      himage=strtol(optarg,&end,10);
      size_arg=true;
      break;
    case 'p':
      // get numbered images from path
      // strncpy(path_value_local,optarg,PATH_BUFFER_SIZE);
      path_value=std::string(optarg);
      break;
    case 'f':
      text_filename=std::string(optarg);
      file_arg=true;
      break;
    case 'c':
      // only cache images
      do_cache=true;
      break;
    case 'd':
      // use database
      use_cache=true;
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
      successful=false;
      return;
    default:
      successful=false;
      return;
    }
  }
  // test for error conditions such as mutually exclusive arguments
  if (size_arg && file_arg) {
    successful=false;
    return;
  } else {
    successful=true;
  }
  path_value=std::string(path_value);
  // get any files on the end
  if (optind != argc) {
    if (path_value[0] != 0) {
      ERROR("Cannot specify both a path and individual files.");
    } else {
      for (auto i=optind; i < argc; i++) {
        filenames.push_back(std::string(argv[i]));
      }
    }
  }
}
