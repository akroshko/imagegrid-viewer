/**
 * Header for the classes to setup the grid to be viewed.  Right now
 * just reads in command line arguments.  Will be suplemented by other
 * sources of grid information and setup in the future.
 */
#ifndef GRIDSETUP_HPP
#define GRIDSETUP_HPP

#include "../common.hpp"
#include "../coordinates.hpp"
#include "../viewport_current_state.hpp"
#include "iterators.hpp"
// C++ headers
#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <vector>

enum class GridSetupStatus {
  not_loaded,
  load_error,
  loaded
};

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
  /** @return The status of whether things loaded properly. */
  GridSetupStatus status() const;
  /**
   * The size of the grid loaded.
   *
   * @return The size of the loaded grid.
   */
  GridImageSize grid_image_size() const;
  /**
   * Indicate whether to cache images.
   *
   * @return Whether to cache images.
   */
  bool setup_cache() const;
  /**
   * Indicate whether to try to use cached images.
   *
   * @return Whether to try to use cached images.
   */
  bool use_cache() const;
  // The items allow access to the underlying data.
  /** @return The size of the imagegrid. */
  GridImageSize grid_size() const;
  /**
   * Check if a grid square has data.
   *
   * @param grid_index The index of the grid square.
   * @return Whether the grid square has data.
   */
  bool square_has_data(const GridIndex& grid_index) const;
  /**
   * Check if a grid square has data.
   *
   * @param grid_index The index of the grid square.
   * @return Whether the grid square has data.
   */
  bool square_has_data(const GridIndex* grid_index) const;
  /**
   * Get the size of the subgrid for a grid square.
   *
   * @param grid_index The index of the grid square.
   * @return The size of the subgrid.
   */
  SubGridImageSize sub_size(const GridIndex& grid_index) const;
  /**
   * Check if a particular subgrid square has.
   *
   * @param grid_index The index of the grid square.
   * @param subgrid_index The subgrid index.
   * @return Whether the subgrid has data.
   */
  bool subgrid_has_data(const GridIndex& grid_index,
                        const SubGridIndex& sub_index) const;
  /**
   * Get filename associated with a particular subgrid square has.
   *
   * @param grid_index The index of the grid square.
   * @param subgrid_index The subgrid index.
   * @return The filename.
   */
  std::string filename(const GridIndex& grid_index,
                           const SubGridIndex& sub_index) const;
  // getting iterators
  std::unique_ptr<ImageGridIteratorVisible> iterator_visible(const ViewPortCurrentState& viewport_current_state);
  std::unique_ptr<ImageGridIteratorFull> iterator_full(const ViewPortCurrentState& viewport_current_state);
protected:
  friend class ImageGridBasicIterator;
  friend class ImageSubGridBasicIterator;
  void _post_setup();
  INT64 _grid_index(INT64 i, INT64 j) const;
  INT64 _grid_index(const GridIndex& grid_index) const;
  INT64 _grid_index(const GridIndex* grid_index) const;
  INT64 _sub_index(INT64 sub_i, INT64 sub_j, INT64 sub_w) const;
  INT64 _sub_index(const SubGridIndex& sub_index, INT64 sub_w) const;
  std::atomic<GridSetupStatus> _status {GridSetupStatus::not_loaded};
  GridImageSize _grid_image_size;
  std::vector<std::string> _filenames;
  std::string _text_filename;
  /** Stores a path of images to load.
   */
  std::string _path_value;
  /** For future expansion. */
  // char _data_set[PATH_BUFFER_SIZE]={ 0 };
  bool _setup_cache=false;
  bool _use_cache=false;
  // some underlying data
  std::unique_ptr<SubGridImageSize[]> _sub_size;
  std::unique_ptr<bool[]> _existing;
  std::list<GridSetupFile> _read_data;
  std::unique_ptr<std::unique_ptr<std::string[]>[]> _file_data;
  // objects to return for const iterators
  std::unique_ptr<GridIndex[]> _grid_index_values;
  // objects to return for const iterators
  std::unique_ptr<std::unique_ptr<SubGridIndex[]>[]> _subgrid_index_values;
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

/**
 * Iterate over all grid squares in a normal order.
 */
class ImageGridBasicIterator {
public:
  ImageGridBasicIterator(GridSetup* grid_setup);
  const GridIndex* begin() const;
  const GridIndex* end() const;
private:
  GridSetup* _grid_setup=nullptr;
};

/**
 * Iterate over all subgrid squares for a particular grid square.
 */
class ImageSubGridBasicIterator {
public:
  ImageSubGridBasicIterator(GridSetup* grid_setup, const GridIndex& grid_index);
  const SubGridIndex* begin() const;
  const SubGridIndex* end() const;
private:
  GridSetup* _grid_setup=nullptr;
  GridIndex _grid_index;
};

#endif
