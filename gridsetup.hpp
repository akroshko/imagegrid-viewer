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

/**
 * Contains the data required to setup the grid.
 */
class GridSetup {
public:
  GridSetup()=default;
  ~GridSetup()=default;
  /** Indicate whether setup was successful. */
  bool successful();
  /** The size of the grid loaded. */
  GridImageSize* grid_image_size;
  /** Stores a path of images to load. */
  std::vector<std::string> filenames;
  // TODO move internally to constructor
  /** Stores a path of images to load. */
  char path_value[256] = { 0 };
protected:
  /** Flag to indicate if setup was successful */
  bool _successful=false;

};

/**
 * Parses command line arguments and returns a set of things to load.
 */
class GridSetupFromCommandLine : public GridSetup {
public:
  GridSetupFromCommandLine(int argc, char* const* argv);
  ~GridSetupFromCommandLine() = default;
};

#endif
