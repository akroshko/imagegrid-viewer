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
#include <list>
#include <memory>
#include <string>
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
  /** Indicate whether setup was successful.
   * @return Setup was successful.
   */
  bool successful() const;
  /** The size of the grid loaded.
   * @return The size of the loaded grid.
   */
  GridImageSize grid_image_size() const;
  /** Indicate whether to cache images.
   *
   * @return Whether to cache images.
   */
  bool do_cache() const;
  /** Indicate whether to try to use cached images.
   *
   * @return Whether to try to use cached images.
   */
  bool use_cache() const;
  // The items allow access to the underlying data.
  INT64 grid_w() const;
  INT64 grid_h() const;
  bool square_has_data(const GridIndex& grid_index) const;
  INT64 subgrid_w(const GridIndex& grid_index) const;
  INT64 subgrid_h(const GridIndex& grid_index) const;
  bool subgrid_has_data(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const;
  std::string get_filename(const GridIndex& grid_index, const SubGridIndex& subgrid_index) const;
protected:
  INT64 _get_grid_index(INT64 i, INT64 j) const;
  INT64 _get_grid_index(const GridIndex& grid_index) const;
  INT64 _get_subgrid_index(INT64 sub_i, INT64 sub_j, INT64 sub_w) const;
  INT64 _get_subgrid_index(const SubGridIndex& subgrid_index, INT64 sub_w) const;
  bool _successful;
  GridImageSize _grid_image_size;
  std::vector<std::string> _filenames;
  std::string _text_filename;
  /** Stores a path of images to load.
   */
  std::string _path_value;
  /** For future expansion. */
  // char _data_set[PATH_BUFFER_SIZE]={ 0 };
  bool _do_cache=false;
  bool _use_cache=false;
  // some underlying data
  std::unique_ptr<INT64[]> _subgrid_width;
  std::unique_ptr<INT64[]> _subgrid_height;
  std::unique_ptr<bool[]> _existing;
  std::list<GridSetupFile> _read_data;
  std::unique_ptr<std::unique_ptr<std::string[]>[]> _new_file_data;
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
