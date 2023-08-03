/**
 * Header for the classes to setup the grid to be viewed.  Right now
 * just reads in command line arguments.  Will be suplemented by other
 * sources of grid information and setup in the future.
 */
#ifndef GRIDSETUP_HPP
#define GRIDSETUP_HPP

#include "debug.hpp"
#include "defaults.hpp"
#include "error.hpp"
#include "types.hpp"
#include "coordinates.hpp"
// C++ headers
#include <vector>
#include <memory>

/**
 * Contains the data required to setup the grid.
 */
class GridSetup {
protected:
  GridSetup()=default;
public:
  ~GridSetup()=default;
  GridSetup(const GridSetup&)=delete;
  GridSetup(const GridSetup&&)=delete;
  GridSetup& operator=(const GridSetup&)=delete;
  GridSetup& operator=(const GridSetup&&)=delete;
  /** Indicate whether setup was successful. */
  bool successful() const;
  /** Returns a path of images to load. */
  std::vector<std::string> filenames() const;
  /** The size of the grid loaded. */
  GridImageSize grid_image_size() const;
protected:
  bool _successful=false;
  GridImageSize _grid_image_size;
  std::vector<std::string> _filenames;
  /** Stores a path of images to load.
   *
   * TODO: change away from a raw string array
   */
  char _path_value[PATH_BUFFER_SIZE]={ 0 };
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
