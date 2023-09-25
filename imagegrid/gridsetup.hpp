/**
 * Header for the classes to setup the grid to be viewed.  Right now
 * just reads in command line arguments.  Will be suplemented by other
 * sources of grid information and setup in the future.
 */
#ifndef GRIDSETUP_HPP
#define GRIDSETUP_HPP

#include "../common.hpp"
#include "../coordinates.hpp"
// C++ headers
#include <vector>

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
  /** Indicate whether to cache images. */
  bool do_cache() const;
  /** Indicate whether to use cached images. */
  bool use_cache() const;
protected:
  bool _successful=false;
  GridImageSize _grid_image_size;
  std::vector<std::string> _filenames;
  /** Stores a path of images to load.
   *
   * TODO: change away from a raw string array
   */
  std::string _path_value;
  /** For future expansion. */
  // char _data_set[PATH_BUFFER_SIZE]={ 0 };
  bool _do_cache=false;
  bool _use_cache=false;
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
