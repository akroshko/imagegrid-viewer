/**
 * Header for the classes to setup the grid to be viewed.  Right now
 * just reads in command line arguments.  Will be suplemented by other
 * sources of grid information and setup in the future.
 */
#ifndef GRIDSETUP_HPP
#define GRIDSETUP_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "coordinates.hpp"

#include <vector>
#include <memory>

/**
 * Contains the data required to setup the grid.
 */
class GridSetup {
public:
  GridSetup()=default;
  ~GridSetup()=default;
  GridSetup(const GridSetup&)=delete;
  GridSetup(const GridSetup&&)=delete;
  GridSetup& operator=(const GridSetup&)=delete;
  GridSetup& operator=(const GridSetup&&)=delete;
  /** Indicate whether setup was successful. */
  bool successful();
  /** Returns a path of images to load. */
  std::vector<std::string> filenames();
  /** The size of the grid loaded. */
  GridImageSize grid_image_size();
protected:
  bool _successful=false;
  GridImageSize _grid_image_size;
  std::vector<std::string> _filenames;
  /** Stores a path of images to load.
   *
   *  This type of program will naturally have a lot of huge path
   *  names, especially for testing.  Max value on Linux?
   */
  char _path_value[4096]={ 0 };
};

/**
 * Parses command line arguments and returns a set of things to load.
 */
class GridSetupFromCommandLine : public GridSetup {
public:
  GridSetupFromCommandLine()=delete;
  GridSetupFromCommandLine(int argc, char* const* argv);
  ~GridSetupFromCommandLine()=default;
  GridSetupFromCommandLine(const GridSetupFromCommandLine&)=delete;
  GridSetupFromCommandLine(const GridSetupFromCommandLine&&)=delete;
  GridSetupFromCommandLine& operator=(const GridSetupFromCommandLine&)=delete;
  GridSetupFromCommandLine& operator=(const GridSetupFromCommandLine&&)=delete;
};

#endif
